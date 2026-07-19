import csv
from pathlib import Path

base_path = Path("results/baseline/summary.csv")
opt_path = Path("results/optimized/summary.csv")
out_path = Path("results/final_cpu_comparison.csv")

rows = []

for path in [base_path, opt_path]:
    with path.open(newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            rows.append(row)

order = {
    "manual": 1,
    "opencv": 2,
    "manual_opt": 3,
    "opencv_opt": 4,
}

rows.sort(key=lambda r: (r["image"], order.get(r["version"], 99)))

with out_path.open("w", newline="") as f:
    fieldnames = [
        "image", "resolution", "version",
        "mean_ms", "median_ms", "min_ms", "max_ms", "std_ms",
        "speedup", "fps",
    ]
    writer = csv.DictWriter(f, fieldnames=fieldnames)
    writer.writeheader()

    for r in rows:
        speedup = ""
        if "speedup_vs_manual" in r:
            speedup = r["speedup_vs_manual"]
        elif "speedup_vs_manual_opt" in r:
            speedup = r["speedup_vs_manual_opt"]

        writer.writerow({
            "image": r["image"],
            "resolution": r["resolution"],
            "version": r["version"],
            "mean_ms": r["mean_ms"],
            "median_ms": r["median_ms"],
            "min_ms": r["min_ms"],
            "max_ms": r["max_ms"],
            "std_ms": r["std_ms"],
            "speedup": speedup,
            "fps": r["fps"],
        })

print(f"written: {out_path}")
