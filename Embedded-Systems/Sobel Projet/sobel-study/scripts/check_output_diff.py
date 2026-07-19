from pathlib import Path
import csv
import cv2

pairs = [
    ("output/baseline/v1_manual", "output/optimized/v3_manual_opt", "manual_vs_manual_opt"),
    ("output/baseline/v2_opencv", "output/optimized/v4_opencv_opt", "opencv_vs_opencv_opt"),
    ("output/baseline/v1_manual", "output/baseline/v2_opencv", "manual_vs_opencv"),
]

out_csv = Path("results/output_diff.csv")

with out_csv.open("w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["comparison", "image", "mean_abs_diff", "max_diff"])

    for a_dir, b_dir, label in pairs:
        a_dir = Path(a_dir)
        b_dir = Path(b_dir)

        if not a_dir.exists() or not b_dir.exists():
            continue

        for a_path in sorted(a_dir.iterdir()):
            if not a_path.is_file():
                continue

            b_path = b_dir / a_path.name
            if not b_path.exists():
                continue

            a = cv2.imread(str(a_path), cv2.IMREAD_GRAYSCALE)
            b = cv2.imread(str(b_path), cv2.IMREAD_GRAYSCALE)

            if a is None or b is None or a.shape != b.shape:
                continue

            diff = cv2.absdiff(a, b)
            writer.writerow([label, a_path.name, f"{float(diff.mean()):.6f}", int(diff.max())])

print(f"written: {out_csv}")
