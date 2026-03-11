library ieee;
use ieee.std_logic_1164.all;

entity Projet3_Nios is
    port (
        CLOCK_50 : in  std_logic;                    -- Clock
        KEY      : in  std_logic_vector(1 downto 0); -- Reset (KEY0)
        SW       : in  std_logic_vector(0 downto 0); -- Sensor (SW0)
        LEDG     : out std_logic_vector(0 downto 0)  -- BOLA (LEDG0) -> زدنا هادي
    );
end entity Projet3_Nios;

architecture rtl of Projet3_Nios is

    component nios_system is
        port (
            clk_clk       : in  std_logic;
            sensor_export : in  std_logic_vector(0 downto 0);
            leds_export   : out std_logic_vector(0 downto 0)  -- الخيط الجديد من Qsys
        );
    end component;

begin

    u0 : component nios_system
        port map (
            clk_clk       => CLOCK_50,
            sensor_export => SW,       -- ربطنا الساروت
            leds_export   => LEDG      -- ربطنا البولة
        );

end architecture rtl;