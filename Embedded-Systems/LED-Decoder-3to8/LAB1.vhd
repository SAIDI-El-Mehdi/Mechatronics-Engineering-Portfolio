library ieee;
use ieee.std_logic_1164.all;

entity LAB1 is
    port (
        SW    : in  std_logic_vector(2 downto 0); -- SW2, SW1, SW0
        LEDG  : out std_logic_vector(7 downto 0)  -- LEDG0 à LEDG7
    );
end LAB1;

architecture LEDs_Controller of LAB1 is
begin
    process(SW)
    begin
        case SW is
            when "000" => LEDG <= "00000001";  -- LEDG0
            when "001" => LEDG <= "00000010";  -- LEDG1
            when "010" => LEDG <= "00000100";  -- LEDG2
            when "011" => LEDG <= "00001000";  -- LEDG3
            when "100" => LEDG <= "00010000";  -- LEDG4
            when "101" => LEDG <= "00100000";  -- LEDG5
            when "110" => LEDG <= "01000000";  -- LEDG6
            when "111" => LEDG <= "10000000";  -- LEDG7
            when others => LEDG <= "00000000"; -- Undefined
        end case;
    end process;
end LEDs_Controller;