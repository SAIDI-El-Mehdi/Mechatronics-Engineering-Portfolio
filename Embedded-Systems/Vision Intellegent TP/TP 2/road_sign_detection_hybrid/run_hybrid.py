#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Hybrid launcher:
- If a YOLO model exists, run the Python YOLO branch.
- Otherwise, run the compiled C++ classical branch.

Usage:
    python run_hybrid.py --input images/input --output images/output --model python_yolo/models/best.pt
"""

from pathlib import Path
import argparse
import subprocess
import sys


def main():
    parser = argparse.ArgumentParser(description="Hybrid road-sign detection launcher")
    parser.add_argument("--input", default="images/input", help="Input image or folder")
    parser.add_argument("--output", default="images/output", help="Output root")
    parser.add_argument("--model", default="python_yolo/models/best.pt", help="YOLO model path")
    parser.add_argument("--cpp-exe", default="cpp_classic/build/detect_classic.exe", help="Compiled C++ executable path")
    args = parser.parse_args()

    model = Path(args.model)
    output = Path(args.output)
    output.mkdir(parents=True, exist_ok=True)

    if model.exists():
        print("[HYBRID] YOLO model found. Using Python YOLO branch.")
        cmd = [
            sys.executable,
            "python_yolo/detect_yolo.py",
            "--model", str(model),
            "--input", args.input,
            "--output", str(output / "yolo"),
        ]
    else:
        print("[HYBRID] YOLO model not found. Using classical C++ OpenCV branch.")
        exe = Path(args.cpp_exe)
        if not exe.exists():
            print(f"[ERROR] C++ executable not found: {exe}")
            print("Compile it first with:")
            print("    cd cpp_classic")
            print("    g++ -std=c++17 detect_classic.cpp -o build/detect_classic.exe $(pkg-config --cflags --libs opencv4)")
            return 1
        cmd = [str(exe), "--input", args.input, "--output", str(output / "classic")]

    print("[HYBRID] Command:")
    print(" ".join(cmd))
    return subprocess.call(cmd)


if __name__ == "__main__":
    sys.exit(main())
