# Autonomous Vehicle Path Tracking & Safety Logic 🚗⚡

## Project Overview
This project demonstrates a complete **Model-Based Design (MBD)** workflow for an autonomous vehicle navigation system. It integrates path planning, trajectory tracking, and safety decision logic using **MATLAB** and **Simulink**.

The system generates a collision-free path in a complex environment and utilizes a **Pure Pursuit controller** to guide a vehicle model (3DOF) along the trajectory. A custom safety logic mechanism ensures the vehicle stops precisely at the goal coordinates.

## 🚀 Key Features
* **Path Planning:** Implemented **Hybrid A*** algorithm (via Navigation Toolbox) to generate smooth, kinematic-feasible paths avoiding static obstacles.
* **Vehicle Dynamics:** Utilized a **3DOF Bicycle Model** (Vehicle Dynamics Blockset) to simulate realistic lateral and longitudinal behavior.
* **Control Strategy:** Designed a **Pure Pursuit Controller** for accurate path following.
* **Safety Logic:** Integrated a Simulink-based decision logic to monitor distance-to-goal and trigger **Autonomous Emergency Braking (AEB)** upon mission completion.

## 🛠️ Tech Stack
* **MATLAB R2021a+**
* **Simulink**
* **Automated Driving Toolbox** / **Navigation Toolbox**
* **Vehicle Dynamics Blockset**

## 📊 Results
The simulation shows the vehicle successfully navigating an S-curve trajectory while avoiding obstacles and executing a precise stop at the target `(45, 45)`.

![Final Simulation Result](results/Final_Result_Plot.png)
*(Figure: Comparison of Planned Path (Green) vs. Actual Vehicle Path (Blue). Note the precise stop at the destination due to the safety logic implementation.)*

## 💻 How to Run
1.  Clone the repository.
2.  Open MATLAB and navigate to the project folder.
3.  Run the script `main_autonomous_navigation.m`.
    * This script will generate the map, plan the path, and automatically launch the Simulink simulation.
4.  The final results comparison will be plotted automatically.

## 📂 Project Structure
* `main_autonomous_navigation.m`: Main entry point (Environment setup + Planning + Execution).
* `path_tracking_model.slx`: Simulink model containing the Vehicle Body, Controller, and Safety Logic.
