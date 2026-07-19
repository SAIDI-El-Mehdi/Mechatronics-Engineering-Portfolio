from pathlib import Path
import csv
import cv2

gpu_dir = Path("output/gpu/v5_cuda")
cpu_pairs = [
    (Path("output/baseline/v2_opencv"), "gpu_vs_opencv"),
    (Path("output/optimized/v4_opencv_opt"), "gpu_vs_opencv_opt"),
    (Path("output/baseline/v1_manual"), "gpu_vs_manual"),
]

out_csv = Path("results/gpu/output_diff.csv")
out_csv.parent.mkdir(parents=True, exist_ok=True)

def index_by_stem(folder: Path):
    d = {}
    if not folder.exists():
        return d
    for p in folder.iterdir():
        if p.is_file():
            d[p.stem] = p
    return d

gpu_map = index_by_stem(gpu_dir)

with out_csv.open("w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["comparison", "image", "mean_abs_diff", "max_diff"])

    for cpu_dir, label in cpu_pairs:
        cpu_map = index_by_stem(cpu_dir)
        for stem, gpu_path in sorted(gpu_map.items()):
            if stem not in cpu_map:
                continue

            cpu_path = cpu_map[stem]

            a = cv2.imread(str(gpu_path), cv2.IMREAD_GRAYSCALE)
            b = cv2.imread(str(cpu_path), cv2.IMREAD_GRAYSCALE)

            if a is None or b is None:
                continue
            if a.shape != b.shape:
                continue

            diff = cv2.absdiff(a, b)
            writer.writerow([label, gpu_path.name, f"{float(diff.mean()):.6f}", int(diff.max())])

print(f"written: {out_csv}")
