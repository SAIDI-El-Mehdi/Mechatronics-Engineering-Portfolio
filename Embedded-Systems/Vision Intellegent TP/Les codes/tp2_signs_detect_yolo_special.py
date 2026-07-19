#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
detect_yolo_special.py

YOLO special extension for road_sign_detection_hybrid.

Features:
- processes all images from images/input
- runs YOLO with several image sizes
- uses a low confidence threshold to recover weak detections
- prints confidence percentage directly in the terminal
- saves annotated images
- saves cropped detections
- saves contact sheet
- saves TXT and CSV reports

Recommended command from the project root:

python python_yolo\detect_yolo_special.py --model python_yolo\models\best.pt --input images\input --output images\output\yolo_special --conf 0.10 --imgsz 640 960 1280
"""

from pathlib import Path
import argparse
import csv
import time
from typing import List, Dict, Any

import cv2
import numpy as np

SUPPORTED_EXTENSIONS = {".jpg", ".jpeg", ".png", ".bmp", ".tif", ".tiff"}


def list_images(path: Path) -> List[Path]:
    if path.is_file() and path.suffix.lower() in SUPPORTED_EXTENSIONS:
        return [path]

    if path.is_dir():
        return sorted(
            p for p in path.rglob("*")
            if p.is_file() and p.suffix.lower() in SUPPORTED_EXTENSIONS
        )

    return []


def ensure_dir(path: Path) -> None:
    path.mkdir(parents=True, exist_ok=True)


def box_iou(box_a, box_b) -> float:
    ax1, ay1, ax2, ay2 = box_a
    bx1, by1, bx2, by2 = box_b

    ix1 = max(ax1, bx1)
    iy1 = max(ay1, by1)
    ix2 = min(ax2, bx2)
    iy2 = min(ay2, by2)

    iw = max(0.0, ix2 - ix1)
    ih = max(0.0, iy2 - iy1)
    inter = iw * ih

    area_a = max(0.0, ax2 - ax1) * max(0.0, ay2 - ay1)
    area_b = max(0.0, bx2 - bx1) * max(0.0, by2 - by1)

    union = area_a + area_b - inter
    if union <= 0.0:
        return 0.0

    return inter / union


def custom_nms(detections: List[Dict[str, Any]], iou_threshold: float) -> List[Dict[str, Any]]:
    detections = sorted(detections, key=lambda d: d["confidence"], reverse=True)
    kept = []

    for detection in detections:
        is_duplicate = False

        for kept_detection in kept:
            same_class = detection["class_id"] == kept_detection["class_id"]
            strong_overlap = box_iou(detection["box"], kept_detection["box"]) > iou_threshold

            if same_class and strong_overlap:
                is_duplicate = True
                break

        if not is_duplicate:
            kept.append(detection)

    return kept


def color_from_confidence(confidence: float):
    if confidence >= 0.70:
        return (0, 255, 0)      # green
    if confidence >= 0.40:
        return (0, 200, 255)    # orange
    return (0, 0, 255)          # red


def draw_detection(image, detection: Dict[str, Any]) -> None:
    x1, y1, x2, y2 = [int(v) for v in detection["box"]]
    class_name = detection["class_name"]
    confidence = detection["confidence"]
    percent = confidence * 100.0
    imgsz = detection["imgsz"]

    color = color_from_confidence(confidence)

    cv2.rectangle(image, (x1, y1), (x2, y2), color, 2)

    label = f"{class_name} {percent:.1f}% | imgsz={imgsz}"
    (text_w, text_h), baseline = cv2.getTextSize(
        label, cv2.FONT_HERSHEY_SIMPLEX, 0.55, 2
    )

    y_top = max(0, y1 - text_h - baseline - 8)
    x_right = min(image.shape[1] - 1, x1 + text_w + 8)

    cv2.rectangle(image, (x1, y_top), (x_right, y1), color, -1)
    cv2.putText(
        image,
        label,
        (x1 + 4, max(text_h + 2, y1 - 6)),
        cv2.FONT_HERSHEY_SIMPLEX,
        0.55,
        (255, 255, 255),
        2
    )


def safe_name(text: str) -> str:
    out = []
    for ch in text:
        if ch.isalnum() or ch in ["_", "-", "."]:
            out.append(ch)
        else:
            out.append("_")
    return "".join(out)


def save_crop(original, detection: Dict[str, Any], crop_dir: Path, image_stem: str, index: int) -> None:
    h, w = original.shape[:2]
    x1, y1, x2, y2 = [int(v) for v in detection["box"]]

    x1 = max(0, min(w - 1, x1))
    x2 = max(0, min(w, x2))
    y1 = max(0, min(h - 1, y1))
    y2 = max(0, min(h, y2))

    if x2 <= x1 or y2 <= y1:
        return

    crop = original[y1:y2, x1:x2]
    percent = detection["confidence"] * 100.0

    crop_name = (
        f"{image_stem}_crop_{index:02d}_"
        f"{safe_name(detection['class_name'])}_"
        f"{percent:.1f}percent.png"
    )

    cv2.imwrite(str(crop_dir / crop_name), crop)


def make_contact_sheet(images: List[np.ndarray], labels: List[str], output_path: Path, thumb_width: int = 360) -> None:
    if not images:
        return

    thumbs = []

    for image, label in zip(images, labels):
        h, w = image.shape[:2]
        scale = thumb_width / max(1, w)
        thumb_height = max(1, int(h * scale))

        thumb = cv2.resize(image, (thumb_width, thumb_height))

        label_bar = np.zeros((38, thumb_width, 3), dtype=np.uint8)
        cv2.putText(
            label_bar,
            label[:48],
            (8, 25),
            cv2.FONT_HERSHEY_SIMPLEX,
            0.55,
            (255, 255, 255),
            1
        )

        thumbs.append(np.vstack([label_bar, thumb]))

    max_h = max(t.shape[0] for t in thumbs)
    padded = []

    for thumb in thumbs:
        if thumb.shape[0] < max_h:
            pad = np.full(
                (max_h - thumb.shape[0], thumb.shape[1], 3),
                255,
                dtype=np.uint8
            )
            thumb = np.vstack([thumb, pad])
        padded.append(thumb)

    cols = min(3, len(padded))
    rows = []

    for i in range(0, len(padded), cols):
        row = padded[i:i + cols]
        while len(row) < cols:
            row.append(np.full_like(padded[0], 255))
        rows.append(np.hstack(row))

    sheet = np.vstack(rows)
    cv2.imwrite(str(output_path), sheet)


def main() -> int:
    parser = argparse.ArgumentParser(description="YOLO special road-sign detector")
    parser.add_argument("--model", required=True, help="Path to YOLO model, for example python_yolo/models/best.pt")
    parser.add_argument("--input", default="images/input", help="Input image or folder")
    parser.add_argument("--output", default="images/output/yolo_special", help="Output folder")
    parser.add_argument("--conf", type=float, default=0.10, help="Confidence threshold")
    parser.add_argument("--iou", type=float, default=0.45, help="NMS IoU threshold")
    parser.add_argument("--imgsz", nargs="+", type=int, default=[640, 960, 1280], help="YOLO image sizes to test")
    args = parser.parse_args()

    model_path = Path(args.model)
    input_path = Path(args.input)
    output_dir = Path(args.output)
    annotated_dir = output_dir / "annotated"
    crops_dir = output_dir / "crops"

    ensure_dir(output_dir)
    ensure_dir(annotated_dir)
    ensure_dir(crops_dir)

    if not model_path.exists():
        print(f"[ERROR] YOLO model not found: {model_path}")
        return 1

    if model_path.stat().st_size == 0:
        print(f"[ERROR] YOLO model is empty: {model_path}")
        return 1

    try:
        from ultralytics import YOLO
    except Exception as exc:
        print("[ERROR] ultralytics is not installed.")
        print("Install it with:")
        print("    pip install ultralytics opencv-python")
        print(f"Details: {exc}")
        return 1

    images = list_images(input_path)

    if not images:
        print(f"[ERROR] No image found in: {input_path}")
        return 1

    print("[INFO] Loading YOLO model...")
    print(f"[INFO] Model : {model_path}")
    print(f"[INFO] Input : {input_path}")
    print(f"[INFO] Output: {output_dir}")
    print(f"[INFO] Confidence threshold: {args.conf * 100:.1f}%")
    print(f"[INFO] Image sizes tested: {args.imgsz}")
    print("")

    model = YOLO(str(model_path))

    report_path = output_dir / "terminal_report_yolo_special.txt"
    csv_path = output_dir / "yolo_special_detections.csv"
    contact_images = []
    contact_labels = []

    total_images = 0
    total_detections = 0
    zero_detection_images = []

    with report_path.open("w", encoding="utf-8") as report, csv_path.open("w", newline="", encoding="utf-8") as csv_file:
        writer = csv.writer(csv_file)
        writer.writerow([
            "image",
            "class_id",
            "class_name",
            "confidence",
            "confidence_percent",
            "x1",
            "y1",
            "x2",
            "y2",
            "imgsz"
        ])

        report.write("YOLO SPECIAL DETECTION REPORT\n")
        report.write("=============================\n\n")
        report.write(f"Model: {model_path}\n")
        report.write(f"Input: {input_path}\n")
        report.write(f"Output: {output_dir}\n")
        report.write(f"Confidence threshold: {args.conf * 100:.1f}%\n")
        report.write(f"Image sizes tested: {args.imgsz}\n")
        report.write(f"NMS IoU threshold: {args.iou}\n\n")

        for image_path in images:
            total_images += 1

            original = cv2.imread(str(image_path))
            if original is None:
                print(f"[WARN] Cannot read image: {image_path}")
                report.write(f"[WARN] Cannot read image: {image_path}\n\n")
                continue

            all_detections = []
            timing_by_size = []

            print(f"[INFO] Processing: {image_path}")
            report.write(f"Image: {image_path}\n")

            for size in args.imgsz:
                start = time.perf_counter()
                results = model.predict(
                    source=str(image_path),
                    imgsz=size,
                    conf=args.conf,
                    verbose=False
                )
                elapsed_ms = (time.perf_counter() - start) * 1000.0
                timing_by_size.append((size, elapsed_ms))

                raw_count = 0

                for result in results:
                    names = result.names
                    boxes = result.boxes

                    if boxes is None:
                        continue

                    for box in boxes:
                        class_id = int(box.cls[0].item())
                        confidence = float(box.conf[0].item())
                        x1, y1, x2, y2 = [float(v) for v in box.xyxy[0].tolist()]
                        class_name = names.get(class_id, str(class_id))

                        all_detections.append({
                            "class_id": class_id,
                            "class_name": class_name,
                            "confidence": confidence,
                            "box": [x1, y1, x2, y2],
                            "imgsz": size
                        })

                        raw_count += 1

                print(f"       imgsz={size:<4} raw={raw_count:<2} time={elapsed_ms:.3f} ms")
                report.write(f"  imgsz={size}: raw_detections={raw_count}, time={elapsed_ms:.3f} ms\n")

            detections = custom_nms(all_detections, args.iou)
            total_detections += len(detections)

            annotated = original.copy()

            if detections:
                print(f"       final detections after NMS: {len(detections)}")
                report.write(f"  Final detections after NMS: {len(detections)}\n")

                for idx, detection in enumerate(detections, start=1):
                    draw_detection(annotated, detection)
                    save_crop(original, detection, crops_dir, image_path.stem, idx)

                    x1, y1, x2, y2 = [int(v) for v in detection["box"]]
                    percent = detection["confidence"] * 100.0

                    print(
                        f"       DET {idx}: "
                        f"class={detection['class_name']} | "
                        f"confidence={percent:.1f}% | "
                        f"imgsz={detection['imgsz']} | "
                        f"bbox=({x1},{y1},{x2},{y2})"
                    )

                    report.write(
                        f"  - DET {idx}: "
                        f"class={detection['class_name']}, "
                        f"confidence={percent:.1f}%, "
                        f"imgsz={detection['imgsz']}, "
                        f"bbox=({x1},{y1},{x2},{y2})\n"
                    )

                    writer.writerow([
                        str(image_path),
                        detection["class_id"],
                        detection["class_name"],
                        f"{detection['confidence']:.4f}",
                        f"{percent:.1f}%",
                        x1,
                        y1,
                        x2,
                        y2,
                        detection["imgsz"]
                    ])
            else:
                zero_detection_images.append(str(image_path))
                print("       No YOLO detection for this image.")
                report.write("  -> No YOLO detection for this image.\n")

            fastest = min(timing_by_size, key=lambda x: x[1]) if timing_by_size else (0, 0.0)
            report.write(f"  Fastest inference: imgsz={fastest[0]}, time={fastest[1]:.3f} ms\n\n")

            output_image = annotated_dir / f"{image_path.stem}_yolo_special.png"
            cv2.imwrite(str(output_image), annotated)

            print(f"       saved={output_image}")
            print("")

            contact_images.append(annotated)
            contact_labels.append(f"{image_path.name} | detections={len(detections)}")

    make_contact_sheet(
        contact_images,
        contact_labels,
        output_dir / "contact_sheet_yolo_special.png"
    )

    with report_path.open("a", encoding="utf-8") as report:
        report.write("\nGLOBAL SUMMARY\n")
        report.write("==============\n")
        report.write(f"Total images: {total_images}\n")
        report.write(f"Total detections: {total_detections}\n")
        report.write(f"Images with zero detection: {len(zero_detection_images)}\n")

        if zero_detection_images:
            report.write("\nZero-detection images:\n")
            for image_name in zero_detection_images:
                report.write(f"  - {image_name}\n")

    print("[DONE] YOLO special branch completed.")
    print(f"[INFO] Annotated images: {annotated_dir}")
    print(f"[INFO] Crops: {crops_dir}")
    print(f"[INFO] Contact sheet: {output_dir / 'contact_sheet_yolo_special.png'}")
    print(f"[INFO] Report: {report_path}")
    print(f"[INFO] CSV: {csv_path}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
