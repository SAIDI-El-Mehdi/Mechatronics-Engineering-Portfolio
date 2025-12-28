# Autonomous Vehicle Path Tracking & Planning Simulation

 🚗 Project Overview
This project demonstrates the design and simulation of a lateral control system for an autonomous vehicle using **MATLAB/Simulink**. The goal is to implement a robust path-tracking algorithm that allows a vehicle model to accurately follow a generated trajectory while adhering to kinematic constraints.

 🎯 Objectives
* Path Planning: Utilize the **Hybrid A*** algorithm to generate collision-free paths in a structured environment.
* Path Tracking: Implement the **Pure Pursuit** controller to calculate the steering angle required to follow the planned path.
* Vehicle Dynamics: Simulate the physical behavior of the car using a **3DOF (Three Degrees of Freedom)** bicycle model within Simulink.
* Performance Analysis: Evaluate the tracking error and system stability under various curvatures.

 🛠️ Technologies & Tools
 **MATLAB & Simulink R202x** (Model-Based Design)
 **Automated Driving Toolbox**
 **Vehicle Dynamics Blockset**
 **Control System Design** (PID & Pure Pursuit)

 📂 Repository Structure
 `PathPlanning_Step1.m`: MATLAB initialization script for generating path coordinates and parameters.
 `AEB_Project.slx`: The main Simulink model implementing the control logic and vehicle dynamics.
 `path_tracking_result_final.png.png`: Visualization of the simulation results.

# 📊 Simulation Results
The following plot illustrates the performance of the Pure Pursuit controller. The blue line represents the actual vehicle trajectory, closely tracking the green dashed reference path.

![Path Tracking Result](path_tracking_result_final.png.png)

# 🚀 How to Run
1.  Clone the repository:
    ```bash
    git clone [https://github.com/SAIDI-El-Mehdi/Mechatronics-Engineering-Portfolio.git](https://github.com/SAIDI-El-Mehdi/Mechatronics-Engineering-Portfolio.git)
    ```
2.  Navigate to the project folder `Matlab-Simulink-Control/Autonomous-Vehicle-Project`.
3.  Open MATLAB.
4.  Run the initialization script `PathPlanning_Step1.m` to load the workspace variables.
5.  Open the Simulink model `AEB_Project.slx`.
6.  Run the simulation.

## 🔗 Author
      El Mehdi SAIDI
Mechatronics Engineering  | Autonomous Systems Enthusiast
www.linkedin.com/in/el-mehdi-morri-45a583334
