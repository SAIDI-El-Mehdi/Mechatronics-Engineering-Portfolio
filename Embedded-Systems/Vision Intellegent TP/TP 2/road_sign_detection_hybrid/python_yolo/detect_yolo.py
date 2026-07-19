#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
YOLO branch for road-sign detection.

Usage:
    python python_yolo/detect_yolo.py --model python_yolo/models/best.pt --input images/input --output images/output/yolo

This script uses the ultralytics package:
    pip install ultralytics opencv-python
"""

from pathlib import Path
import argparse
import csv
import sys
import time

import cv2


SUPPORTED_EXT = {".jpg", ".jpeg", ".png", ".bmp", ".tif", ".tiff"}


def list_images(input_path: Path):
    if input_path.is_file() and input_path.suffix.lower() in SUPPORTED_EXT:
        return [input_path]
    if input_path.is_dir():
        return sorted([p for p in input_path.rglob("*") if p.is_file() and p.suffix.lower() in SUPPORTED_EXT])
    return []


def make_output_name(output_dir: Path, image_path: Path, suffix: str):
    output_dir.mkdir(parents=True, exist_ok=True)
    return output_dir / f"{image_path.stem}_{suffix}.png"


def main():
    parser = argparse.ArgumentParser(description="YOLO road-sign detection branch")
    parser.add_argument("--model", required=True, help="Path to YOLO model, for example python_yolo/models/best.pt")
    parser.add_argument("--input", default="images/input", help="Input image or folder")
    parser.add_argument("--output", default="images/output/yolo", help="Output folder")
    parser.add_argument("--conf", type=float, default=0.25, help="Confidence threshold")
    args = parser.parse_args()

    model_path = Path(args.model)
    input_path = Path(args.input)
    output_dir = Path(args.output)
    output_dir.mkdir(parents=True, exist_ok=True)

    if not model_path.exists():
        print(f"[ERROR] YOLO model not found: {model_path}")
        print("[INFO] Put your model in python_yolo/models/, for example best.pt")
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
    model = YOLO(str(model_path))

    report_path = output_dir / "terminal_report_yolo.txt"
    csv_path = output_dir / "yolo_detections.csv"

    with report_path.open("w", encoding="utf-8") as report, csv_path.open("w", newline="", encoding="utf-8") as csv_file:
        writer = csv.writer(csv_file)
        writer.writerow(["image", "class_id", "class_name", "confidence", "x1", "y1", "x2", "y2"])

        report.write("YOLO ROAD-SIGN DETECTION REPORT\n")
        report.write("================================\n\n")
        report.write(f"Model: {model_path}\n")
        report.write(f"Input: {input_path}\n")
        report.write(f"Output: {output_dir}\n")
        report.write(f"Confidence threshold: {args.conf}\n\n")

        for image_path in images:
            print(f"[INFO] Processing: {image_path}")
            image = cv2.imread(str(image_path))
            if image is None:
                print(f"[WARN] Cannot read: {image_path}")
                continue

            t0 = time.perf_counter()
            results = model.predict(source=str(image_path), conf=args.conf, verbose=False)
            infer_ms = (time.perf_counter() - t0) * 1000.0

            annotated = image.copy()
            detection_count = 0

            report.write(f"Image: {image_path}\n")
            report.write(f"  Inference time: {infer_ms:.3f} ms\n")

            for result in results:
                names = result.names
                boxes = result.boxes
                if boxes is None:
                    continue

                for box in boxes:
                    cls_id = int(box.cls[0].item())
                    conf = float(box.conf[0].item())
                    x1, y1, x2, y2 = [int(v) for v in box.xyxy[0].tolist()]
                    class_name = names.get(cls_id, str(cls_id))

                    detection_count += 1
                    cv2.rectangle(annotated, (x1, y1), (x2, y2), (0, 255, 0), 2)
                    label = f"{class_name} {conf:.2f}"
                    cv2.rectangle(annotated, (x1, max(0, y1 - 24)), (x1 + 220, y1), (0, 255, 0), -1)
                    cv2.putText(annotated, label, (x1 + 5, max(18, y1 - 6)),
                                cv2.FONT_HERSHEY_SIMPLEX, 0.55, (0, 0, 0), 2)

                    writer.writerow([str(image_path), cls_id, class_name, f"{conf:.4f}", x1, y1, x2, y2])
                    report.write(
                        f"  - class={class_name}, conf={conf:.3f}, "
                        f"bbox=({x1},{y1},{x2},{y2})\n"
                    )

            report.write(f"  Total detections: {detection_count}\n\n")

            output_file = make_output_name(output_dir, image_path, "yolo_annotated")
            cv2.imwrite(str(output_file), annotated)
            print(f"       detections={detection_count}, time={infer_ms:.3f} ms, saved={output_file}")

    print("[DONE] YOLO branch completed.")
    print(f"[INFO] Report: {report_path}")
    print(f"[INFO] CSV: {csv_path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
