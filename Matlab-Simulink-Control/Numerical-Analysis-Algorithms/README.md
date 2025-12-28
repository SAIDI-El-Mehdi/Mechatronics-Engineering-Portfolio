# Numerical Analysis Algorithms Collection

## 🧮 Overview
This repository contains a collection of MATLAB scripts implementing fundamental numerical methods used in engineering for solving equations, interpolation, and differential equations. These algorithms demonstrate the core concepts of scientific computing.

## 📂 Included Algorithms

### 1. Newton-Raphson Method (`Newton.m`)
* **Description:** A root-finding algorithm that produces successively better approximations to the roots (or zeroes) of a real-valued function.
* **Key Features:**
    * Uses derivative information ($f'(x)$) for faster convergence.
    * Includes iteration tracking and tolerance stopping criteria ($\epsilon$).

### 2. Runge-Kutta 4th Order (`RK4.m`)
* **Description:** Implements the classic RK4 method for solving Ordinary Differential Equations (ODEs).
* **Application:** Used for simulating dynamic systems where high accuracy is required compared to the Euler method.
* **Math:** Solves $y' = f(x, y)$ using a weighted average of four increments.

### 3. Lagrange Interpolation (`Lagrange.m`)
* **Description:** Constructs a polynomial that passes through a given set of data points $(x, y)$.
* **Application:** Useful for data fitting and estimating values between known discrete points.

### 4. Linear System Solver (`Resolution.m`)
* **Description:** Solves systems of linear equations of the form $Ax = b$.
* **Features:**
    * Calculates the determinant to check for singularity.
    * Computes the solution vector $X$ using matrix division.

## 🚀 How to Run
1.  Open any script in MATLAB.
2.  Ensure the parameters (initial conditions, functions) are defined at the top of the script.
3.  Run the script to see the numerical output in the Command Window.

## 🔗 Author
**El Mehdi SAIDI**
Mechatronics Engineering 
