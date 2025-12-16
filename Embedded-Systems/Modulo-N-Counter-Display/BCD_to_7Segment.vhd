library IEEE;
use ieee.std_logic_1164.all;

entity BCD_to_7Segment is
    Port (
        BCD      : in std_logic_vector(3 downto 0);  -- 4-bit BCD input
        Segments : out std_logic_vector(6 downto 0) -- 7-segment output (a to g)
    );
end BCD_to_7Segment;

architecture Behavioral of BCD_to_7Segment is
begin
    process(BCD)
    begin
        case BCD is
            when "0000" => Segments <= "1000000"; -- 0
            when "0001" => Segments <= "1111001"; -- 1
            when "0010" => Segments <= "0100100"; -- 2
            when "0011" => Segments <= "0110000"; -- 3
            when "0100" => Segments <= "0011001"; -- 4
            when "0101" => Segments <= "0010010"; -- 5
            when "0110" => Segments <= "0000010"; -- 6
            when "0111" => Segments <= "1111000"; -- 7
            when "1000" => Segments <= "0000000"; -- 8
            when "1001" => Segments <= "0010000"; -- 9
            when others => Segments <= "1111111"; -- Blank (invalid input)
        end case;
    end process;
end Behavioral;