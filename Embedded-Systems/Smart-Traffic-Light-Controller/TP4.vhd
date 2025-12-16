library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity TP4 is
    port(
        CLOCK_50 : in std_logic;      -- Horloge 50MHz
        BUTTON   : in std_logic_vector(2 downto 0); -- Reset (BUTTON[0])
        SW       : in std_logic_vector(9 downto 0); -- Les Switches
        LEDG     : out std_logic_vector(9 downto 0) -- Les LEDs (Sorties)
    );
end entity;

architecture Structural of TP4 is

    -- 1. Déclaration des Composants (Li sawbna 9bel)
    component Timer
        generic(c_MAX : integer := 50000000); -- 1 Seconde réelle
        port(Clk, Rst : in std_logic; Tick_1s : out std_logic);
    end component;

    component FSM_Auto
        port(
            Clk, Rst, Tick_1s : in std_logic;
            R1, Y1, G1, R2, Y2, G2 : out std_logic
        );
    end component;

    -- 2. Signaux Internes
    signal s_Tick : std_logic; -- Khyet mabin Timer w FSM
    signal s_Rst  : std_logic;
    
    -- Signaux li jayin mn FSM (Mode Auto)
    signal auto_R1, auto_Y1, auto_G1 : std_logic;
    signal auto_R2, auto_Y2, auto_G2 : std_logic;

    -- Signal pour Clignotement (Standby)
    signal blink_state : std_logic := '0';

    -- Alias pour les Switches (Bach n-sahhlo l-9raya)
    alias ON_SW    : std_logic is SW(0);
    alias Auto_SW  : std_logic is SW(1);
    alias Man_SW   : std_logic is SW(2);
    alias Stdby_SW : std_logic is SW(3);
    
    -- Commandes Manuelles (Exemple d'assignation)
    alias Man_Pass_1 : std_logic is SW(4);
    alias Man_Stop_1 : std_logic is SW(5);
    alias Man_Pass_2 : std_logic is SW(6);
    alias Man_Stop_2 : std_logic is SW(7);

begin
    -- Reset inversé (hit boutons f DE0 Active Low, walakin hna nsta3mluh normal)
    s_Rst <= not BUTTON(0); 

    -- 3. Instanciation Timer
    U_Timer : Timer 
        port map(Clk => CLOCK_50, Rst => s_Rst, Tick_1s => s_Tick);

    -- 4. Instanciation FSM (Mode Auto)
    U_FSM : FSM_Auto 
        port map(
            Clk => CLOCK_50, Rst => s_Rst, Tick_1s => s_Tick,
            R1 => auto_R1, Y1 => auto_Y1, G1 => auto_G1,
            R2 => auto_R2, Y2 => auto_Y2, G2 => auto_G2
        );

    -- 5. Process pour Clignotement (Standby - 1s ON / 1s OFF)
    process(CLOCK_50, s_Rst)
    begin
        if s_Rst = '1' then
            blink_state <= '0';
        elsif rising_edge(CLOCK_50) then
            if s_Tick = '1' then
                blink_state <= not blink_state; -- Change l-hala kolla taniya
            end if;
        end if;
    end process;

    -- 6. Multiplexeur Principal (Le Cerveau qui décide)
    -- Hada howa li kaytbe9 Table de Vérité (Page 1)
    process(ON_SW, Auto_SW, Man_SW, Stdby_SW, auto_R1, auto_Y1, auto_G1, blink_state, SW)
    begin
        -- Initialisation à 0 (Par défaut tout est éteint)
        LEDG <= (others => '0'); 

        if ON_SW = '1' then
            -- Vérifier Mode Auto (Priorité ou Exclusivité)
            if (Auto_SW='1' and Man_SW='0' and Stdby_SW='0') then
                -- Brancher les LEDs sur le FSM Auto
                LEDG(0) <= auto_G1; LEDG(1) <= auto_Y1; LEDG(2) <= auto_R1;
                LEDG(5) <= auto_G2; LEDG(6) <= auto_Y2; LEDG(7) <= auto_R2;

            -- Vérifier Mode Manuel
            elsif (Auto_SW='0' and Man_SW='1' and Stdby_SW='0') then
                -- Logique Manuelle (Page 2) - Exemple simple
                if Man_Pass_1 = '1' then LEDG(0) <= '1'; -- G1
                elsif Man_Stop_1 = '1' then LEDG(1) <= '1'; -- Y1 (Simplifié)
                else LEDG(2) <= '1'; -- R1 par défaut
                end if;
                
                if Man_Pass_2 = '1' then LEDG(5) <= '1'; -- G2
                elsif Man_Stop_2 = '1' then LEDG(6) <= '1'; -- Y2
                else LEDG(7) <= '1'; -- R2
                end if;

            -- Vérifier Mode Standby
            elsif (Auto_SW='0' and Man_SW='0' and Stdby_SW='1') then
                -- Jaune clignote
                LEDG(1) <= blink_state; -- Y1
                LEDG(6) <= blink_state; -- Y2
            
            else
                -- Cas d'erreur (Plusieurs switchs ON) => Tout éteint
                LEDG <= (others => '0');
            end if;
        else
            -- Si ON_SW = 0 => Tout éteint
            LEDG <= (others => '0');
        end if;
    end process;

end Structural;