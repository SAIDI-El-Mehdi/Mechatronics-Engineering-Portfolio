library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity FSM_Auto is
    port(
        Clk     : in std_logic;
        Rst     : in std_logic;
        Tick_1s : in std_logic;  -- Signal li jay mn Timer
        
        -- Les Sorties (Commandes des LEDs)
        -- R1, Y1, G1 = Tri9 1
        -- R2, Y2, G2 = Tri9 2
        R1, Y1, G1 : out std_logic;
        R2, Y2, G2 : out std_logic
    );
end entity;

architecture Behavioral of FSM_Auto is

    -- Hadou homa les étapes li f'TP (Page 2)
    type State_Type is (Etape1, Etape2, Etape3, Etape4, Etape5, Etape6);
    signal current_state : State_Type := Etape1;
    
    -- Compteur interne bach n7sbu 30s w 3s
    signal timer_count : integer range 0 to 35 := 0;

begin

    process(Clk, Rst)
    begin
        if Rst = '1' then
            current_state <= Etape1;
            timer_count   <= 0;
        elsif rising_edge(Clk) then
            -- Ntsennaw hta tji Taniya (Tick_1s)
            if Tick_1s = '1' then
                timer_count <= timer_count + 1;
                
                case current_state is
                    -- Etape 1: Green 1 (30s)
                    when Etape1 =>
                        if timer_count >= 30 then 
                            current_state <= Etape2;
                            timer_count <= 0;
                        end if;

                    -- Etape 2: Yellow 1 (3s)
                    when Etape2 =>
                        if timer_count >= 3 then 
                            current_state <= Etape3;
                            timer_count <= 0;
                        end if;

                    -- Etape 3: Red 1 & Red 2 (3s) - Sécurité
                    when Etape3 =>
                        if timer_count >= 3 then 
                            current_state <= Etape4;
                            timer_count <= 0;
                        end if;
                        
                    -- Etape 4: Green 2 (30s)
                    when Etape4 =>
                        if timer_count >= 30 then 
                            current_state <= Etape5;
                            timer_count <= 0;
                        end if;
                        
                    -- Etape 5: Yellow 2 (3s)
                    when Etape5 =>
                        if timer_count >= 3 then 
                            current_state <= Etape6;
                            timer_count <= 0;
                        end if;
                        
                    -- Etape 6: Red 1 & Red 2 (3s)
                    when Etape6 =>
                        if timer_count >= 3 then 
                            current_state <= Etape1; -- Retour au début
                            timer_count <= 0;
                        end if;
                end case;
            end if;
        end if;
    end process;

    -- Sorties (Logic combinatoire simple)
    -- Hna kanch3lu l-boulat 3la hssab l-Etape
    process(current_state)
    begin
        -- Par défaut ntfiu kolchi bach mannsawch chi haja
        R1 <= '0'; Y1 <= '0'; G1 <= '0';
        R2 <= '0'; Y2 <= '0'; G2 <= '0';
        
        case current_state is
            when Etape1 =>
                G1 <= '1'; R2 <= '1'; -- Vert 1, Rouge 2
            when Etape2 =>
                Y1 <= '1'; R2 <= '1'; -- Jaune 1, Rouge 2
            when Etape3 =>
                R1 <= '1'; R2 <= '1'; -- Rouge 1, Rouge 2
            when Etape4 =>
                R1 <= '1'; G2 <= '1'; -- Rouge 1, Vert 2
            when Etape5 =>
                R1 <= '1'; Y2 <= '1'; -- Rouge 1, Jaune 2
            when Etape6 =>
                R1 <= '1'; R2 <= '1'; -- Rouge 1, Rouge 2
        end case;
    end process;

end Behavioral;