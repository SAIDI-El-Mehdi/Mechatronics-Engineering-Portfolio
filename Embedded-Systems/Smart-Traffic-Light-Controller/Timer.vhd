library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity Timer is
    generic(
        -- 50,000,000 = 1 Seconde (l'horloge d-carte fiha 50MHz)
        c_MAX : integer := 50000000 
    );
    port(
        Clk     : in std_logic;      -- Horloge
        Rst     : in std_logic;      -- Reset
        Tick_1s : out std_logic      -- Signal 1 Seconde
    );
end entity;

architecture Behavioral of Timer is
    signal count : integer range 0 to c_MAX := 0;
begin

    process(Clk, Rst)
    begin
        if Rst = '1' then
            count <= 0;
            Tick_1s <= '0';
        elsif rising_edge(Clk) then
            if count = c_MAX - 1 then
                Tick_1s <= '1';      -- Signal kaych3el mlli katzdouz taniya
                count <= 0;
            else
                Tick_1s <= '0';
                count <= count + 1;
            end if;
        end if;
    end process;

end Behavioral;