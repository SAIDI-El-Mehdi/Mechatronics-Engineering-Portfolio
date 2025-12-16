library IEEE;
  use IEEE.std_logic_1164.all;

  entity add_1_bit is
    port (
      x: in std_logic;
      y: in std_logic;
      cin: in std_logic;
      sum: out std_logic;
      cout: out std_logic
    );
  end add_1_bit;

  architecture rtl of add_1_bit is
  begin
    sum <= x xor y xor cin;
    cout <= (x and y) or (x and cin) or (y and cin);
  end rtl;