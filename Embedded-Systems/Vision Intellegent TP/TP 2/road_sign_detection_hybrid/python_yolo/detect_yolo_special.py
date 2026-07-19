#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
YOLO Special Extension for road_sign_detection_hybrid.

Main features:
- runs YOLO on all images in images/input
- uses a lower confidence threshold by default
- tests several image sizes to recover missed detections
- merges duplicate detections with custom NMS
- saves annotated images
- saves cropped detections
- saves a comparison/contact sheet
- saves CSV + TXT report

Recommended command from C:\road_sign_detection_hybrid:

python python_yolo\detect_yolo_special.py ^
  --model python_yolo\models\best.pt ^
  --input images\input ^
  --output images\output\yolo_special ^
  --conf 0.10 ^
  --imgsz 640 960 1280
"""

from pathlib import Path
import argparse
import csv
import time
from typing import List, Dict, Any

import cv2
import numpy as np

SUPPORTED_EXT = {".jpg", ".jpeg", ".png", ".bmp", ".tif", ".tiff"}


def list_images(path: Path) -> List[Path]:
    if path.is_file() and path.suffix.lower() in SUPPORTED_EXT:
        return [path]
    if path.is_dir():
        return sorted([p for p in path.rglob("*") if p.is_file() and p.suffix.lower() in SUPPORTED_EXT])
    return []


def ensure_dir(path: Path) -> None:
    path.mkdir(parents=True, exist_ok=True)


def box_iou(a, b) -> float:
    ax1, ay1, ax2, ay2 = a
    bx1, by1, bx2, by2 = b
    inter_x1 = max(ax1, bx1)
    inter_y1 = max(ay1, by1)
    inter_x2 = min(ax2, bx2)
    inter_y2 = min(ay2, by2)

    iw = max(0, inter_x2 - inter_x1)
    ih = max(0, inter_y2 - inter_y1)
    inter = iw * ih

    area_a = max(0, ax2 - ax1) * max(0, ay2 - ay1)
    area_b = max(0, bx2 - bx1) * max(0, by2 - by1)
    union = area_a + area_b - inter

    if union <= 0:
        return 0.0
    return inter / union


def custom_nms(detections: List[Dict[str, Any]], iou_threshold: float = 0.45) -> List[Dict[str, Any]]:
    detections = sorted(detections, key=lambda d: d["conf"], reverse=True)
    kept = []

    for det in detections:
        duplicate = False
        for kept_det in kept:
            if det["class_id"] == kept_det["class_id"] and box_iou(det["box"], kept_det["box"]) > iou_threshold:
                duplicate = True
                break
        if not duplicate:
            kept.append(det)

    return kept


def confidence_color(conf: float):
    if conf >= 0.70:
        return (0, 255, 0)       # green
    if conf >= 0.40:
        return (0, 200, 255)     # orange
    return (0, 0, 255)           # red


def draw_detection(image, det: Dict[str, Any]):
    x1, y1, x2, y2 = [int(v) for v in det["box"]]
    conf = det["conf"]
    class_name = det["class_name"]
    imgsz = det["imgsz"]

    color = confidence_color(conf)
    cv2.rectangle(image, (x1, y1), (x2, y2), color, 2)

    label = f"{class_name} {conf:.2f} | {imgsz}"
    (tw, th), baseline = cv2.getTextSize(label, cv2.FONT_HERSHEY_SIMPLEX, 0.55, 2)
    y_text_top = max(0, y1 - th - baseline - 6)
    cv2.rectangle(image, (x1, y_text_top), (min(image.shape[1] - 1, x1 + tw + 6), y1), color, -1)
    cv2.putText(image, label, (x1 + 3, max(th + 2, y1 - 5)),
                cv2.FONT_HERSHEY_SIMPLEX, 0.55, (255, 255, 255), 2)


def save_crop(original, det: Dict[str, Any], crop_dir: Path, image_stem: str, index: int):
    x1, y1, x2, y2 = [int(v) for v in det["box"]]
    h, w = original.shape[:2]

    x1 = max(0, min(w - 1, x1))
    x2 = max(0, min(w, x2))
    y1 = max(0, min(h - 1, y1))
    y2 = max(0, min(h, y2))

    if x2 <= x1 or y2 <= y1:
        return

    crop = original[y1:y2, x1:x2]
    out_name = f"{image_stem}_crop_{index:02d}_{det['class_name']}_{det['conf']:.2f}.png"
    out_name = out_name.replace(" ", "_").replace("/", "_").replace("\\", "_")
    cv2.imwrite(str(crop_dir / out_name), crop)


def make_contact_sheet(images: List[np.ndarray], labels: List[str], output_path: Path, thumb_w: int = 360):
    if not images:
        return

    thumbs = []
    for img, label in zip(images, labels):
        h, w = img.shape[:2]
        scale = thumb_w / max(1, w)
        thumb_h = int(h * scale)
        thumb = cv2.resize(img, (thumb_w, thumb_h))

        label_area = np.zeros((34, thumb_w, 3), dtype=np.uint8)
        cv2.putText(label_area, label[:45], (8, 23), cv2.FONT_HERSHEY_SIMPLEX, 0.55, (255, 255, 255), 1)
        thumb = np.vstack([label_area, thumb])
        thumbs.append(thumb)

    max_h = max(t.shape[0] for t in thumbs)
    padded = []
    for t in thumbs:
        if t.shape[0] < max_h:
            pad = np.full((max_h - t.shape[0], t.shape[1], 3), 255, dtype=np.uint8)
            t = np.vstack([t, pad])
        padded.append(t)

    cols = min(3, len(padded))
    rows = []
    for i in range(0, len(padded), cols):
        row_imgs = padded[i:i + cols]
        while len(row_imgs) < cols:
            row_imgs.append(np.full_like(padded[0], 255))
        rows.append(np.hstack(row_imgs))

    sheet = np.vstack(rows)
    cv2.imwrite(str(output_path), sheet)


def main():
    parser = argparse.ArgumentParser(description="Special YOLO road-sign detection extension")
    parser.add_argument("--model", required=True, help="Path to YOLO model, for example python_yolo/models/best.pt")
    parser.add_argument("--input", default="images/input", help="Input image or folder")
    parser.add_argument("--output", default="images/output/yolo_special", help="Output folder")
    parser.add_argument("--conf", type=float, default=0.10, help="Low confidence threshold for candidate recovery")
    parser.add_argument("--iou", type=float, default=0.45, help="Custom NMS IoU threshold")
    parser.add_argument("--imgsz", nargs="+", type=int, default=[640, 960, 1280], help="Image sizes to test")
    args = parser.parse_args()

    model_path = Path(args.model)
    input_path = Path(args.input)
    output_dir = Path(args.output)
    annotated_dir = output_dir / "annotated"
    crops_dir = output_dir / "crops"

    ensure_dir(output_dir)
    ensure_dir(annotated_dir)
    ensure_dir(crops_dir)

    if not model_path.exists() or model_path.stat().st_size == 0:
        print(f"[ERROR] YOLO model missing or empty: {model_path}")
        return 1

    try:
        from ultralytics import YOLO
    except Exception as exc:
        print("[ERROR] ultralytics is not installed.")
        print("Install with:")
        print("    pip install ultralytics opencv-python")
        print(f"Details: {exc}")
        return 1

    images = list_images(input_path)
    if not images:
        print(f"[ERROR] No input images found in: {input_path}")
        return 1

    print("[INFO] Loading YOLO model...")
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
            "image", "class_id", "class_name", "confidence",
            "x1", "y1", "x2", "y2", "imgsz", "source"
        ])

        report.write("YOLO SPECIAL DETECTION REPORT\n")
        report.write("=============================\n\n")
        report.write(f"Model: {model_path}\n")
        report.write(f"Input: {input_path}\n")
        report.write(f"Output: {output_dir}\n")
        report.write(f"Confidence threshold: {args.conf}\n")
        report.write(f"Image sizes tested: {args.imgsz}\n")
        report.write(f"NMS IoU threshold: {args.iou}\n\n")

        for image_path in images:
            total_images += 1
            original = cv2.imread(str(image_path))
            if original is None:
                print(f"[WARN] Cannot read image: {image_path}")
                continue

            all_detections = []
            inference_times = []

            print(f"[INFO] Processing: {image_path}")
            report.write(f"Image: {image_path}\n")

            for size in args.imgsz:
                t0 = time.perf_counter()
                results = model.predict(source=str(image_path), imgsz=size, conf=args.conf, verbose=False)
                infer_ms = (time.perf_counter() - t0) * 1000.0
                inference_times.append((size, infer_ms))

                count_this_size = 0

                for result in results:
                    names = result.names
                    boxes = result.boxes
                    if boxes is None:
                        continue

                    for box in boxes:
                        cls_id = int(box.cls[0].item())
                        conf = float(box.conf[0].item())
                        x1, y1, x2, y2 = [float(v) for v in box.xyxy[0].tolist()]
                        class_name = names.get(cls_id, str(cls_id))

                        all_detections.append({
                            "class_id": cls_id,
                            "class_name": class_name,
                            "conf": conf,
                            "box": [x1, y1, x2, y2],
                            "imgsz": size
                        })
                        count_this_size += 1

                report.write(f"  imgsz={size}: raw_detections={count_this_size}, time={infer_ms:.3f} ms\n")

            detections = custom_nms(all_detections, args.iou)
            total_detections += len(detections)

            annotated = original.copy()
            for idx, det in enumerate(detections, start=1):
                draw_detection(annotated, det)
                save_crop(original, det, crops_dir, image_path.stem, idx)

                x1, y1, x2, y2 = [int(v) for v in det["box"]]
                writer.writerow([
                    str(image_path), det["class_id"], det["class_name"], f"{det['conf']:.4f}",
                    x1, y1, x2, y2, det["imgsz"], "YOLO_SPECIAL"
                ])

                report.write(
                    f"  - DET {idx}: class={det['class_name']}, conf={det['conf']:.3f}, "
                    f"bbox=({x1},{y1},{x2},{y2}), imgsz={det['imgsz']}\n"
                )

            if not detections:
                zero_detection_images.append(str(image_path))
                report.write("  -> No detection after multi-scale YOLO.\n")

            out_file = annotated_dir / f"{image_path.stem}_yolo_special.png"
            cv2.imwrite(str(out_file), annotated)

            contact_images.append(annotated)
            contact_labels.append(f"{image_path.name} | det={len(detections)}")

            best_time = min(inference_times, key=lambda x: x[1]) if inference_times else (0, 0)
            report.write(f"  Final detections after NMS: {len(detections)}\n")
            report.write(f"  Fastest inference: imgsz={best_time[0]}, time={best_time[1]:.3f} ms\n\n")

            print(f"       detections={len(detections)}, saved={out_file}")

    make_contact_sheet(contact_images, contact_labels, output_dir / "contact_sheet_yolo_special.png")

    with report_path.open("a", encoding="utf-8") as report:
        report.write("\nGLOBAL SUMMARY\n")
        report.write("==============\n")
        report.write(f"Total images: {total_images}\n")
        report.write(f"Total detections: {total_detections}\n")
        report.write(f"Images with zero detection: {len(zero_detection_images)}\n")
        if zero_detection_images:
            report.write("\nZero-detection images:\n")
            for p in zero_detection_images:
                report.write(f"  - {p}\n")

    print("[DONE] YOLO special branch completed.")
    print(f"[INFO] Annotated images: {annotated_dir}")
    print(f"[INFO] Crops: {crops_dir}")
    print(f"[INFO] Contact sheet: {output_dir / 'contact_sheet_yolo_special.png'}")
    print(f"[INFO] Report: {report_path}")
    print(f"[INFO] CSV: {csv_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
