# 3-to-8 Line Decoder (VHDL)

## 📌 Overview
This project implements a **3-to-8 binary decoder** using VHDL. The module takes 3 input switches as a binary address and activates one of the 8 output LEDs accordingly. It demonstrates the use of **combinatorial logic** and the `CASE` statement for hardware description.

## ⚙️ Features
* **Input:** 3-bit vector (representing switches SW2-SW0).
* **Output:** 8-bit vector (driving LEDs LED7-LED0).
* **Logic Style:** Behavioral modeling using `PROCESS` and `CASE` statements.

## 🛠️ Tools Used
* **Software:** Intel Quartus Prime (Lite Edition).
* **Simulation:** ModelSim / QuestaSim.
* **Hardware:** (Optional: Mention your board, e.g., DE10-Lite FPGA).

## 📊 Simulation Waveform
*(Important: Add a screenshot of your ModelSim simulation here showing the inputs changing and outputs reacting)*
![Simulation Result](path_to_your_image.png)

## 📝 Code Snippet
Key logic implementation:
```vhdl
process(switches)
begin
    case switches is
        when "000" => leds <= "00000001";
        when "001" => leds <= "00000010";
        -- ... other cases
        when others => leds <= "00000000";
    end case;
end process;
