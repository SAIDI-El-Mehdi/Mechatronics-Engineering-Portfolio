 Artificial Respirator Mechanism (Respirateur Artificiel)
Design of a mechanical ventilator system intended for emergency respiratory support.
The mechanism converts rotary motion from a DC motor into a linear compression stroke to actuate an Ambu bag.

 📸 Assembly Views
Simulation of the compression mechanism:

[Respirator Assembly](./respirator_views.png)

 ⚙️ System Components
- Actuation: DC Motor with reduction gear.
- Transmission: Cam & Follower / Crank-Slider mechanism.
- Support: Frame designed to hold the manual resuscitator (Ambu bag).
  
 🫁 Mechanical Artificial Respirator Design

An advanced mechanical design project for an artificial ventilator aimed at assisting patients with respiratory insufficiency.

---

 📖 Project Overview
In the context of the global demand for medical equipment (COVID-19), this project focuses on the design and study of a **mechanical artificial respirator**. The device reproduces the human respiratory cycle using a mechanical system that converts rotary motion into a reciprocating motion to compress an insufflation bag (Ambu bag).

* Institution: ENSA Agadir (École Nationale des Sciences Appliquées)
* Field: Mechatronics & Automotive Technology
* Tools Used: CATIA V5, SolidWorks, Mathematical Modeling

 ⚙️ Mechanical Concept (System Architecture)
The core mechanism is based on a **Crank-Connecting Rod System (Système Bielle-Manivelle)**. This system transforms the continuous rotation of an asynchronous motor into a linear alternating motion to drive the pusher (platine).

 Key Components:
1.  Asynchronous Motor: The main power source.
2.  Crank-Rod Mechanism: Converts rotary motion to linear motion.
3.  Pusher (Platine): Compresses the respiratory bag.
4.  Guiding System: Ensures precise linear translation.
5.  Chassis: Structural support for all components.

 📐 Technical Specifications & Calculations
Based on the kinematic study performed in the report:

| Parameter | Value | Description |
| :--- | :--- | :--- |
| Crank Radius (r) | 20 mm | Defines the amplitude of the motion |
| Rod Length (L) | 100 mm | Connects crank to pusher |
| Rotation Speed (N) | 50 tr/min | Motor speed |
| Total Stroke (C) | 40 mm | Max displacement (2  r) |
| Useful Volume (V) | 48,000 mm^3 | Air volume displaced per cycle |
| Avg. Airflow (Q) | 4 *10^{-5} m^3/s | Calculated average airflow |

> Mathematical Model:
> The instantaneous position of the pusher X_p(t) and the airflow Q(t) were modeled to ensure a smooth respiratory cycle.
> X_p(t) = r \cdot \cos(\theta) + L \sqrt{1 - (\frac{r}{L})^2 \sin^2(\theta)}

 💻 CAD Design (CATIA V5 & SolidWorks)
The mechanical assembly was fully modeled to validate dimensions and constraints.
* CATIA V5: Used for detailed part design (Pusher, Screw, Guide).
* SolidWorks: Used for assembly verification and rendering.

 📸 Gallery
 👥 Authors & Acknowledgments
* Realized by: El Mehdi SAIDI & Zouhair Elouaggadi
* Supervisor: Pr. L. BELARCHE
* Context: 2nd Year Mechatronics Engineering - Project "Conception Avancée" (2025/2026)
