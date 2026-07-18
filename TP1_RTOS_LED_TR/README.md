\# ⏱️ Real-Time Operating Systems (RTOS): STM32 LED Blink



This directory contains practical lab work (TP1) focused on embedded Real-Time Operating Systems (RTOS). The project is developed using \*\*STM32CubeIDE\*\* and demonstrates basic task scheduling and hardware control using \*\*FreeRTOS\*\*.



\## 🎯 Project Overview

The primary objective of this project is to implement a real-time multitasking environment on an STM32 microcontroller. It features an RTOS-managed LED blink application, ensuring precise timing and resource management independent of bare-metal delay loops.



\## 🛠️ Hardware \& Tech Stack

\*   \*\*Microcontroller:\*\* STM32 (Target: STM32F446RETX)

\*   \*\*Framework/Middleware:\*\* FreeRTOS

\*   \*\*Development Environment:\*\* STM32CubeIDE / HAL Drivers

\*   \*\*Language:\*\* C (Embedded)



\## 📂 Project Structure

\*   `Core/`: Contains main application code, initialization, and RTOS task definitions.

\*   `Drivers/`: Hardware Abstraction Layer (HAL) drivers for STM32 peripherals.

\*   `Middlewares/`: FreeRTOS source files and core kernel logic.

\*   `TP1\_RTOS\_LED\_Blink.ioc`: STM32CubeMX configuration file for pinouts and clock tree setup.

