# Modulo-N Counter with 7-Segment Display

## 📟 Project Overview
This project implements a flexible **Modulo-N Counter** on an FPGA using VHDL. It counts from 0 to a user-defined limit ($N$) and displays the current count on a 7-segment display (HEX). Ideally suited for creating digital clocks, timers, or event counters.

## ⚙️ Key Features
* **Dynamic Modulo Limit:** The count limit $N$ can be set via input switches.
* **BCD Conversion:** Includes a binary-to-BCD converter (`BinToBCD.vhd`) to properly format numbers for the display.
* **Display Control:** A dedicated driver (`BCD_to_7segment.vhd`) to handle the 7-segment encoding.
* **Frequency Divider:** (If applicable) Scaled down the 50MHz clock to 1Hz for visible counting.

## 📂 File Structure
* `modulo_n_counter.vhd`: Top-level entity integrating the counter and display logic.
* `BinToBCD.vhd`: Module to convert binary values to Binary-Coded Decimal.
* `BCD_to_7segment.vhd`: Decoder logic for the 7-segment display.
* `ModuloCounter_Project.qpf`: Intel Quartus Prime project file.

## 🛠️ Hardware & Tools
* **Language:** VHDL (IEEE 1076)
* **IDE:** Intel Quartus Prime
* **Hardware:** (Mention your board, e.g., DE1-SoC, DE10-Lite)
