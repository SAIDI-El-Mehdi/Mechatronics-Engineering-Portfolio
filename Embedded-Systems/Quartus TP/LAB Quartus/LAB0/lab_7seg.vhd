library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity lab_7seg is
    port (
        dec : in  std_logic_vector(3 downto 0);
        seg : out std_logic_vector(6 downto 0)
    );
end lab_7seg;

architecture Behavioral of lab_7seg is
begin
    seg <= "1111110" when dec = "0000" else  -- 0
           "0110000" when dec = "0001" else  -- 1
           "1101101" when dec = "0010" else  -- 2
           "1111001" when dec = "0011" else  -- 3
           "0110011" when dec = "0100" else  -- 4
           "1011011" when dec = "0101" else  -- 5
           "1011111" when dec = "0110" else  -- 6
           "1110000" when dec = "0111" else  -- 7
           "1111111" when dec = "1000" else  -- 8
           "1111011" when dec = "1001" else  -- 9
           "0000000";                        
end;