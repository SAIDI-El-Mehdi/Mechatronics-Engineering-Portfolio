library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity TB_LAB1 is
end TB_LAB1;

architecture TEST of TB_LAB1 is

    -- Declaration of Entity(Unite) Unde Test (UUT)
    component LAB1
        port (
            SW   : in  std_logic_vector(2 downto 0);
            LEDG : out std_logic_vector(7 downto 0)
        );
    end component;

    -- Internal signals for test
    signal SW_Test   : std_logic_vector(2 downto 0) := "000"; -- Initial value: "000"
    signal LEDG_Test : std_logic_vector(7 downto 0);

begin

    -- Instantiation of LAB1
    UUT: LAB1
        port map (
            SW   => SW_Test,
            LEDG => LEDG_Test
        );

    -- Stimulation : SW_Test change value every 100ns
    process
    begin
        for i in 0 to 7 loop
            SW_Test <= std_logic_vector(to_unsigned(i, 3));
            wait for 100 ns;
        end loop;
        wait;
    end process;

end TEST;