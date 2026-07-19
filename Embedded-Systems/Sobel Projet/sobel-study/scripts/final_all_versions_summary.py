import csv
from pathlib import Path

cpu_src = Path("results/global_cpu_summary.csv")
gpu_src = Path("results/gpu/summary.csv")
out = Path("results/final_all_versions_summary.csv")

rows = []

with cpu_src.open(newline="") as f:
    reader = csv.DictReader(f)
    for r in reader:
        rows.append({
            "version": r["version"],
            "mean_ms": float(r["global_mean_ms"])
        })

gpu_means = []
with gpu_src.open(newline="") as f:
    reader = csv.DictReader(f)
    for r in reader:
        gpu_means.append(float(r["mean_total_ms"]))

if gpu_means:
    rows.append({
        "version": "cuda_gpu",
        "mean_ms": sum(gpu_means) / len(gpu_means)
    })

rows.sort(key=lambda r: {
    "manual": 1,
    "opencv": 2,
    "manual_opt": 3,
    "opencv_opt": 4,
    "cuda_gpu": 5
}.get(r["version"], 99))

with out.open("w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["version", "global_mean_ms", "global_fps"])
    for r in rows:
        fps = 1000.0 / r["mean_ms"] if r["mean_ms"] > 0 else 0.0
        writer.writerow([r["version"], f"{r['mean_ms']:.6f}", f"{fps:.2f}"])

print(f"written: {out}")
