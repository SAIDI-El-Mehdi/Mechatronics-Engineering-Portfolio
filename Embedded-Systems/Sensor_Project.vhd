library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity Sensor_Project is
    port (
        clk          : in  std_logic;                    -- Clock 50 MHz
        sensor_input : in  std_logic;                    
        led_out      : out std_logic;                    
        data_out     : out std_logic_vector(7 downto 0)
    );
end entity Sensor_Project;

architecture rtl of Sensor_Project is
    
    signal counter : unsigned(7 downto 0) := (others => '0');
    signal last_state : std_logic := '0';
begin

    process(clk)
    begin
        if rising_edge(clk) then
            
            last_state <= sensor_input;

            
            if (last_state = '0' and sensor_input = '1') then
                counter <= counter + 1; 
                led_out <= '1';         
            else
                led_out <= '0';         
            end if;
        end if;
    end process;

    
    data_out <= std_logic_vector(counter);

end architecture rtl;