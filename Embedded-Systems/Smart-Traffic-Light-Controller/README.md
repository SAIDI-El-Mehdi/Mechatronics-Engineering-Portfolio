# Smart Traffic Light Controller (FSM Based)

## 🚦 Project Overview
This project involves the design and implementation of a **Smart Traffic Light Controller** using VHDL. The core logic is built around a **Finite State Machine (FSM)** that manages state transitions (Red, Green, Yellow) based on timing constraints and external sensor inputs (Automatic vs. Manual modes).

## 🧠 Core Architecture
The system is modularized into three main components:
1.  **FSM Controller (`FSM_Auto.vhd`):** The brain of the system. It defines the state diagram and transitions based on current conditions.
2.  **Timer Module (`Timer.vhd`):** A precise clock divider/counter that generates timing signals (e.g., 30s for Green, 3s for Yellow).
3.  **Top-Level Integration (`TrafficLight_Top.vhd`):** Interconnects the FSM and Timer to drive the physical traffic lights.

## ⚙️ Features
* **Dual Mode Operation:**
    * *Automatic Mode:* Standard timed cycle.
    * *Manual Mode:* User overrides via switches/buttons.
* **Synchronous Design:** All state changes are synchronized with the system clock.
* **Safety Logic:** Default failsafe states (e.g., blinking yellow or all red) handled by the FSM.

## 📂 File Structure
* `TrafficLight_Top.vhd`: Main system entry point.
* `FSM_Auto.vhd`: Finite State Machine logic.
* `Timer.vhd`: Timing generation module.
* `tb_TrafficLight.vhd`: VHDL Testbench for verifying state transitions.

## 🛠️ Tools
* **Language:** VHDL (IEEE 1076)
* **FPGA Board:** (e.g., Intel/Altera DE10-Lite)
* **Simulation:** ModelSim / QuestaSim
