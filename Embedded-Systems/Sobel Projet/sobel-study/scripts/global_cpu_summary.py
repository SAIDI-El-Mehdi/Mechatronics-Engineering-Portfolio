import csv
from collections import defaultdict
from pathlib import Path

src = Path("results/final_cpu_comparison.csv")
out = Path("results/global_cpu_summary.csv")

groups = defaultdict(list)

with src.open(newline="") as f:
    reader = csv.DictReader(f)
    for row in reader:
        groups[row["version"]].append(float(row["mean_ms"]))

with out.open("w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["version", "global_mean_ms"])

    order = ["manual", "opencv", "manual_opt", "opencv_opt"]
    for version in order:
        vals = groups.get(version, [])
        if vals:
            writer.writerow([version, f"{sum(vals)/len(vals):.6f}"])

print(f"written: {out}")
