library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity testbench is
end entity testbench;

architecture behavior of testbench is

    -- نعيطو على المشروع ديالك
    component Sensor_Project is
        port (
            clk          : in  std_logic;
            sensor_input : in  std_logic;
            led_out      : out std_logic;
            data_out     : out std_logic_vector(7 downto 0)
        );
    end component;

    -- خيوط باش نربطو
    signal clk_tb          : std_logic := '0';
    signal sensor_input_tb : std_logic := '0';
    signal led_out_tb      : std_logic;
    signal data_out_tb     : std_logic_vector(7 downto 0);

    -- توقيت الساعة (Clock Period)
    constant clk_period : time := 20 ns; -- 50MHz

begin

    -- نربطو الخيوط مع المشروع
    uut: Sensor_Project port map (
        clk          => clk_tb,
        sensor_input => sensor_input_tb,
        led_out      => led_out_tb,
        data_out     => data_out_tb
    );

    -- 1. نصاوبو Clock (كتبقى تضرب ديما)
    clk_process : process
    begin
        clk_tb <= '0';
        wait for clk_period/2;
        clk_tb <= '1';
        wait for clk_period/2;
    end process;

    -- 2. نصاوبو الكابتور (كنلعبو بيه)
    stim_proc: process
    begin
        wait for 100 ns;
        
        -- الكابتور شاف شي حاجة (Daba kayn signal)
        sensor_input_tb <= '1';
        wait for 50 ns;
        sensor_input_tb <= '0'; -- مشا
        wait for 50 ns;

        -- شاف مرة اخرى
        sensor_input_tb <= '1';
        wait for 100 ns;
        sensor_input_tb <= '0';
        
        wait; -- سالينا
    end process;

end architecture;