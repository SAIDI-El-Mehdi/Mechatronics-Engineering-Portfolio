library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity tb_controllerTraffic is
end tb_controllerTraffic;

architecture sim of tb_controllerTraffic is

    -- Component declaration
    component controllerTraffic
        Port (
            clk            : in  STD_LOGIC;
            reset          : in  STD_LOGIC;
            Manual_pass_1  : in  STD_LOGIC;
            Manual_stop_1  : in  STD_LOGIC;
            Manual_fstop_1 : in  STD_LOGIC;
            Manual_pass_2  : in  STD_LOGIC;
            Manual_stop_2  : in  STD_LOGIC;
            Manual_fstop_2 : in  STD_LOGIC;
            SW_MODE        : in  std_logic_vector(3 downto 0);
            LED_red_1      : out STD_LOGIC;
            LED_yellow_1   : out STD_LOGIC;
            LED_green_1    : out STD_LOGIC;
            LED_red_2      : out STD_LOGIC;
            LED_yellow_2   : out STD_LOGIC;
            LED_green_2    : out STD_LOGIC;
            LED_MODE       : out std_logic_vector(3 downto 0)
        );
    end component;

    -- Signals
    signal clk            : STD_LOGIC := '0';
    signal reset          : STD_LOGIC := '0';
    signal Manual_pass_1  : STD_LOGIC := '0';
    signal Manual_stop_1  : STD_LOGIC := '0';
    signal Manual_fstop_1 : STD_LOGIC := '0';
    signal Manual_pass_2  : STD_LOGIC := '0';
    signal Manual_stop_2  : STD_LOGIC := '0';
    signal Manual_fstop_2 : STD_LOGIC := '0';
    signal SW_MODE        : std_logic_vector(3 downto 0) := (others => '0');
    signal LED_red_1, LED_yellow_1, LED_green_1 : STD_LOGIC;
    signal LED_red_2, LED_yellow_2, LED_green_2 : STD_LOGIC;
    signal LED_MODE : std_logic_vector(3 downto 0);

    -- Clock generation: 50 MHz -> period = 20 ns
    constant clk_period : time := 20 ps;
begin
    -- Instantiate the Unit Under Test (UUT)
    uut: controllerTraffic
        port map (
            clk            => clk,
            reset          => reset,
            Manual_pass_1  => Manual_pass_1,
            Manual_stop_1  => Manual_stop_1,
            Manual_fstop_1 => Manual_fstop_1,
            Manual_pass_2  => Manual_pass_2,
            Manual_stop_2  => Manual_stop_2,
            Manual_fstop_2 => Manual_fstop_2,
            SW_MODE        => SW_MODE,
            LED_red_1      => LED_red_1,
            LED_yellow_1   => LED_yellow_1,
            LED_green_1    => LED_green_1,
            LED_red_2      => LED_red_2,
            LED_yellow_2   => LED_yellow_2,
            LED_green_2    => LED_green_2,
            LED_MODE       => LED_MODE
        );

    -- Clock process
    clk_process : process
    begin
        clk <= '0';
        wait for clk_period / 2;
        clk <= '1';
        wait for clk_period / 2;
    end process;

    -- Stimulus process
    stim_proc: process
    begin
        -- Reset pulse
        reset <= '1';
        wait for 50 ns;
        reset <= '0';

        -- Test AUTO mode (SW_MODE = "1100")
        SW_MODE <= "1100";
        wait for 2 us;

        -- Test MANUAL mode
        SW_MODE <= "1010";
        Manual_pass_1 <= '1';
        wait for 100 ns;
        Manual_pass_1 <= '0';

        Manual_stop_1 <= '1';
        wait for 200 ns;
        Manual_stop_1 <= '0';

        Manual_fstop_1 <= '1';
        wait for 100 ns;
        Manual_fstop_1 <= '0';

        -- Test MANUAL direction 2
        Manual_pass_2 <= '1';
        wait for 100 ns;
        Manual_pass_2 <= '0';

        Manual_stop_2 <= '1';
        wait for 200 ns;
        Manual_stop_2 <= '0';

        Manual_fstop_2 <= '1';
        wait for 100 ns;
        Manual_fstop_2 <= '0';

        -- Test STANDBY mode
        SW_MODE <= "1000";
        wait for 2 us;

        -- End simulation
        wait;
    end process;

end sim;
