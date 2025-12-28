library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity tb_TrafficLight is
end entity;

architecture behavior of tb_TrafficLight is

    -- Component Declaration for the Unit Under Test (UUT)
    component tb_TrafficLight
        port(
            CLOCK_50 : in std_logic;
            BUTTON   : in std_logic_vector(2 downto 0);
            SW       : in std_logic_vector(9 downto 0);
            LEDG     : out std_logic_vector(9 downto 0)
        );
    end component;

    -- Inputs
    signal clk : std_logic := '0';
    signal btn : std_logic_vector(2 downto 0) := "111"; -- (1 = Not pressed)
    signal sw  : std_logic_vector(9 downto 0) := (others => '0');

    -- Outputs
    signal led : std_logic_vector(9 downto 0);

    -- Clock period definitions
    constant clk_period : time := 20 ns; -- 50 MHz

begin

    -- Instantiate the Unit Under Test (UUT)
    uut: tb_TrafficLight port map (
        CLOCK_50 => clk,
        BUTTON   => btn,
        SW       => sw,
        LEDG     => led
    );

    -- Clock Process (Kaybqa yderb dima)
    clk_process :process
    begin
        clk <= '0'; wait for clk_period/2;
        clk <= '1'; wait for clk_period/2;
    end process;

    -- Stimulus Process (Scenario d-Test)
    stim_proc: process
    begin		
        -- 1. Initialisation (Reset)
        btn(0) <= '0'; -- Wrek 3la Reset
        wait for 100 ns;
        btn(0) <= '1'; -- Tle9 Reset
        wait for 20 ns;

        -- 2. Test Standby (Clignotement Sfer)
        sw(0) <= '1'; -- ON_SW
        sw(3) <= '1'; -- Stdby_SW
        wait for 200 ns; -- Khassk tchouf LEDG(1) w LEDG(6) kay-clignotiw

        -- 3. Test Manuel
        sw(3) <= '0'; -- Tfi Standby
        sw(2) <= '1'; -- Cha3el Manuel (Man_SW)
        wait for 40 ns;
        
        -- Tri9 1 Douz
        sw(4) <= '1'; -- Man_Pass_1
        wait for 100 ns;
        sw(4) <= '0';
        
        -- Tri9 2 Douz
        sw(6) <= '1'; -- Man_Pass_2
        wait for 100 ns;
        sw(6) <= '0';

        sw(2) <= '0'; -- Tfi Manuel

        -- 4. Test Automatique
        sw(1) <= '1'; -- Cha3el Auto (Auto_SW)
        -- Hna ghadi ntsennaw cycle kaml
        -- Hit beddelna Timer l 2, hadchi ghaydouz bzerba
        wait for 1000 ns; 

        wait; -- Sali Test
    end process;


end behavior;
