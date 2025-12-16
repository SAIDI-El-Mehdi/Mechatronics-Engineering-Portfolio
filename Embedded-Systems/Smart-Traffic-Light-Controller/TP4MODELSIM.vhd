library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity TP4MODELSIM is
    port(
        CLOCK_50 : in std_logic;
        BUTTON   : in std_logic_vector(2 downto 0);
        SW       : in std_logic_vector(9 downto 0);
        LEDG     : out std_logic_vector(9 downto 0)
    );
end entity;

architecture Structural of TP4MODELSIM is

    component Timer
        generic(c_MAX : integer := 50000000);
        port(Clk, Rst : in std_logic; Tick_1s : out std_logic);
    end component;

    component FSM_Auto
        port(
            Clk, Rst, Tick_1s : in std_logic;
            R1, Y1, G1, R2, Y2, G2 : out std_logic
        );
    end component;

    signal s_Tick : std_logic;
    signal s_Rst  : std_logic;
    
    signal auto_R1, auto_Y1, auto_G1 : std_logic;
    signal auto_R2, auto_Y2, auto_G2 : std_logic;

    signal blink_state : std_logic := '0';

    alias ON_SW    : std_logic is SW(0);
    alias Auto_SW  : std_logic is SW(1);
    alias Man_SW   : std_logic is SW(2);
    alias Stdby_SW : std_logic is SW(3);
    
    alias Man_Pass_1 : std_logic is SW(4);
    alias Man_Stop_1 : std_logic is SW(5);
    alias Man_Pass_2 : std_logic is SW(6);
    alias Man_Stop_2 : std_logic is SW(7);

begin
    s_Rst <= not BUTTON(0); 

    -- HNA FIN BEDDELNA L-MAGANA L 10 (Simulation)
    U_Timer : Timer 
        generic map (c_MAX => 10)
        port map(Clk => CLOCK_50, Rst => s_Rst, Tick_1s => s_Tick);

    U_FSM : FSM_Auto 
        port map(
            Clk => CLOCK_50, Rst => s_Rst, Tick_1s => s_Tick,
            R1 => auto_R1, Y1 => auto_Y1, G1 => auto_G1,
            R2 => auto_R2, Y2 => auto_Y2, G2 => auto_G2
        );

    process(CLOCK_50, s_Rst)
    begin
        if s_Rst = '1' then
            blink_state <= '0';
        elsif rising_edge(CLOCK_50) then
            if s_Tick = '1' then
                blink_state <= not blink_state;
            end if;
        end if;
    end process;

    process(ON_SW, Auto_SW, Man_SW, Stdby_SW, auto_R1, auto_Y1, auto_G1, blink_state, SW)
    begin
        LEDG <= (others => '0'); 

        if ON_SW = '1' then
            if (Auto_SW='1' and Man_SW='0' and Stdby_SW='0') then
                LEDG(0) <= auto_G1; LEDG(1) <= auto_Y1; LEDG(2) <= auto_R1;
                LEDG(5) <= auto_G2; LEDG(6) <= auto_Y2; LEDG(7) <= auto_R2;

            elsif (Auto_SW='0' and Man_SW='1' and Stdby_SW='0') then
                if Man_Pass_1 = '1' then LEDG(0) <= '1';
                elsif Man_Stop_1 = '1' then LEDG(1) <= '1';
                else LEDG(2) <= '1';
                end if;
                
                if Man_Pass_2 = '1' then LEDG(5) <= '1';
                elsif Man_Stop_2 = '1' then LEDG(6) <= '1';
                else LEDG(7) <= '1';
                end if;

            elsif (Auto_SW='0' and Man_SW='0' and Stdby_SW='1') then
                LEDG(1) <= blink_state;
                LEDG(6) <= blink_state;
            
            else
                LEDG <= (others => '0');
            end if;
        else
            LEDG <= (others => '0');
        end if;
    end process;

end Structural;