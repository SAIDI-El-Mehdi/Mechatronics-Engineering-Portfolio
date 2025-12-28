# 4-Bit Binary Adder with 7-Segment Display

## 🔢 Project Overview
This project implements a **4-bit Ripple Carry Adder** using a structural VHDL design approach. Instead of using high-level arithmetic operators, the adder is constructed by cascading four instances of a **1-bit Full Adder**. The result is processed and displayed on a 7-Segment HEX display.

## 🏗️ Design Architecture
* **Structural Design:** The top-level entity instantiates four 1-bit Full Adders connected in a chain (Carry-Out to Carry-In).
* **Modular Components:**
    * `1_bit_Full_Adder.vhd`: Basic logic block (Sum & Carry calculation).
    * `Adder4Bit_Top.vhd`: Top-level wrapper connecting the adders and mapping outputs.
* **Display Logic:** Includes BCD decoding to visualize the summation result on hardware.

## 📂 File Structure
* `Adder4Bit_Top.vhd`: Main structural entity.
* `1_bit_Full_Adder.vhd`: The fundamental building block.
* `tb_Adder4Bit.vhd`: Testbench for verifying addition logic and carry propagation.
* `Adder4Bit_Project.qpf`: Intel Quartus Prime project file.

## ⚙️ Hardware & Tools
* **FPGA Board:** (e.g., DE10-Lite / Cyclone V)
* **Synthesis Tool:** Intel Quartus Prime
* **Simulation:** ModelSim
