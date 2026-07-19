library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;  -- Pour les compteurs

entity controllerTraffic is
    Port (
        clk     : in  STD_LOGIC;
        reset   : in  STD_LOGIC;
		  Manual_pass_1 : in STD_LOGIC;
        Manual_stop_1 : in STD_LOGIC;
        Manual_fstop_1 : in STD_LOGIC;
        Manual_pass_2 : in STD_LOGIC;
		  Manual_stop_2 : in STD_LOGIC;
		  Manual_fstop_2 : in STD_LOGIC;
		  SW_MODE : in std_logic_vector(3 downto 0);--ON_SW ,Auto_SW, Man_SW ,Stdby_SW  
        LED_red_1    : out STD_LOGIC;
        LED_yellow_1 : out STD_LOGIC;
        LED_green_1  : out STD_LOGIC;
        LED_red_2    : out STD_LOGIC;
        LED_yellow_2 : out STD_LOGIC;
        LED_green_2  : out STD_LOGIC;
		  LED_MODE : out std_logic_vector(3 downto 0)--LED_ON,LED_Auto,LED_Man,LED_Stdby

    );
end controllerTraffic;

architecture Behavioral of controllerTraffic is

    type      state_type is (etape1, etape2, etape3, etape4, etape5, etape6);
    signal    state : state_type := etape1;
	 type      Sby is (s1, s2);
    signal    etat : Sby := s1;
	 
	 signal    LED_red_1A    :  STD_LOGIC;
    signal    LED_yellow_1A :  STD_LOGIC;
    signal    LED_green_1A  :  STD_LOGIC;
    signal    LED_red_2A    :  STD_LOGIC;
    signal    LED_yellow_2A :  STD_LOGIC;
    signal    LED_green_2A  :  STD_LOGIC;
	 
	 signal    LED_red_1M    :  STD_LOGIC;
    signal    LED_yellow_1M :  STD_LOGIC;
    signal    LED_green_1M  :  STD_LOGIC;
    signal    LED_red_2M    :  STD_LOGIC;
    signal    LED_yellow_2M :  STD_LOGIC;
    signal    LED_green_2M  :  STD_LOGIC;
	 
	 signal    LED_yellow_1S :  STD_LOGIC;
	 signal    LED_yellow_2S :  STD_LOGIC;

    -- Compteur de temporisation
    signal counter_stdby : unsigned(31 downto 0) := (others => '0');
	 signal counter_manual : unsigned(31 downto 0) := (others => '0');
	 signal counter_auto : unsigned(31 downto 0) := (others => '0');
	 --signal counter_manual : unsigned(4 downto 0) := (others => '0');
	 --signal counter_auto : unsigned(4 downto 0) := (others => '0');
	 --signal counter_stdby : unsigned(4 downto 0) := (others => '0');
    --signal delay_done : std_logic := '0';
    signal duration : unsigned(27 downto 0) := (others => '0');

    -- Constantes de durée (à 50 MHz)
    constant DUREE_30S : unsigned(31 downto 0) := to_unsigned(1500000000, 32); -- 30 s
    constant DUREE_3S  : unsigned(31 downto 0) := to_unsigned(150000000, 32);  -- 3 s
	 constant DUREE_1S  : unsigned(31 downto 0) := to_unsigned(50000000, 32);  -- 1 s
	 --constant DUREE_30S : unsigned(4 downto 0) := "11110";
	 --constant DUREE_3S : unsigned(4 downto 0) := "00011";
	 --constant DUREE_2S : unsigned(4 downto 0) := "00010";

begin
	
	
	
	process(SW_MODE)
	begin
	case (SW_MODE) is 
		  when "1100"	=>
		  LED_MODE<="1100";
		  when "1010"	=>
		  LED_MODE<="1010";
		  when "1001"	=>
		  LED_MODE<="1001";
		  when "1000"	=>
		  LED_MODE<="1000";
		  when others	=>
		  LED_MODE<="0000";
    end case;		  
	 end process;
			
	 
    Auto :process(clk, reset,SW_MODE,counter_auto)
    begin
	 if SW_MODE="1100" then
        if  reset = '0' then
            state <= etape1;
            counter_auto <= (others => '0');
            --delay_done <= '0';
            --duration <= DUREE_30S;

        elsif rising_edge(clk) then
        -- Gestion du compteur
        case (state) is              --state transitions
         when etape1 =>
           if counter_auto < DUREE_30S then
             state <= etape1;
             counter_auto <= counter_auto + 1;                
           else  
             state <= etape2;
             counter_auto <= (others => '0');
           end if;

         when etape2 =>
           if counter_auto < DUREE_3S then
             state <=  etape2;
             counter_auto <= counter_auto + 1;
           else
             state <= etape3;
             counter_auto <= (others => '0');
           end if;

         when etape3 =>
           if counter_auto < DUREE_3S then
             state <= etape3;
             counter_auto <= counter_auto + 1;
           else
             state <= etape4;
             counter_auto <= (others => '0');
           end if;

         when etape4 =>   
           if counter_auto < DUREE_30S then
             state <=  etape4;
             counter_auto <= counter_auto + 1;
           else
             state <=etape5;
             counter_auto <= (others => '0');
           end if; 

         when etape5 =>
           if counter_auto < DUREE_3S then
             state <=  etape5;
             counter_auto <= counter_auto + 1;
           else
             state <=etape6;
             counter_auto <= (others => '0');
           end if;
         when etape6 =>
           if counter_auto < DUREE_3S then
             state <=  etape6;
             counter_auto <= counter_auto + 1;
           else
             state <=etape1;
             counter_auto <= (others => '0');
           end if;			  
       end case;
       end if;
    else
	 state<=etape1;
	 counter_auto <= (others => '0'); 
	 end if;	 
    end process;
	 
	 Manual : process(clk,SW_MODE,counter_manual,Manual_pass_1,Manual_pass_2,Manual_fstop_1,Manual_fstop_2,Manual_stop_1,Manual_stop_2)
	 begin
	 if SW_MODE = "1010" then
				if Manual_pass_1 = '1' then
                LED_green_1M <= '1';
            else
                LED_green_1M <= '0';
            end if;
            if Manual_stop_1 = '1' then
					 if counter_manual < DUREE_3S then
						LED_yellow_1M <= '1';
                  counter_manual <= counter_manual + 1;
                else
					   counter_manual <= (others => '0');
						LED_yellow_1M <= '0';
                  LED_red_1M <= '1';					 
					 end if;
            else
                LED_yellow_1M <= '0';
            end if;
            if Manual_fstop_1 = '1' then
                LED_red_1M <= '1';
            else
                LED_red_1M <= '0';
            end if;
            -- Direction 2
            if Manual_pass_2 = '1' then
                LED_green_2M <= '1';
            else
                LED_green_2M <= '0';
            end if;
            if Manual_stop_2 = '1' then
					 if counter_manual < DUREE_3S then
						LED_yellow_2M <= '1';
                  counter_manual <= counter_manual + 1;
                else
					   counter_manual <= (others => '0');
						LED_yellow_2M <= '0';
                  LED_red_2M <= '1';
						
					 end if;
            else
                LED_yellow_2M <= '0';
            end if;
            if Manual_fstop_2 = '1' then
                LED_red_2M <= '1';
            else
                LED_red_2M <= '0';
            end if;
    end if;				
	 end process;
	 
	 Standby : process(clk)
		begin
			 if rising_edge(clk) then
				  if SW_MODE = "1001" then
						case etat is
							 when s1 =>
								  if counter_stdby < DUREE_1S then
										counter_stdby <= counter_stdby + 1;
								  else  
										etat <= s2;
										counter_stdby <= (others => '0');
								  end if;

							 when s2 =>
								  if counter_stdby < DUREE_1S then
										counter_stdby <= counter_stdby + 1;
								  else
										etat <= s1;
										counter_stdby <= (others => '0');
								  end if;
						end case;
				  else
						etat <= s1;
						counter_stdby <= (others => '0');
				  end if;
			 end if;
	 end process;
    stadby : process(etat) 
	 begin
	  case etat is
                    when s1 =>
                     LED_yellow_1S <= '1';
				         LED_yellow_2S <= '1';			
                    when s2 =>
						   LED_yellow_1S <= '0';
				         LED_yellow_2S <= '0';
     end case;
	 end process;
	 
	 output:process(state)
	 begin
                -- Changement d'état une fois le temps écoulé
                case state is
                    when etape1 =>
                        LED_green_1A  <= '1';
                        LED_yellow_1A <= '0';
                        LED_red_1A    <= '0';
                        LED_green_2A  <= '0';
                        LED_yellow_2A <= '0';
                        LED_red_2A    <= '1';

                    when etape2 =>
                        LED_green_1A  <= '0';
                        LED_yellow_1A <= '1';
                        LED_red_1A    <= '0';
                        LED_green_2A  <= '0';
                        LED_yellow_2A <= '0';
                        LED_red_2A    <= '1';

                    when etape3 =>
                        LED_green_1A  <= '0';
                        LED_yellow_1A <= '0';
                        LED_red_1A    <= '1';
                        LED_green_2A  <= '0';
                        LED_yellow_2A <= '0';
                        LED_red_2A    <= '1';

                    when etape4 =>
                        LED_green_1A  <= '0';
                        LED_yellow_1A <= '0';
                        LED_red_1A    <= '1';
                        LED_green_2A  <= '1';
                        LED_yellow_2A <= '0';
                        LED_red_2A    <= '0';

                    when etape5 =>
                        LED_green_1A  <= '0';
                        LED_yellow_1A <= '0';
                        LED_red_1A    <= '1';
                        LED_green_2A  <= '0';
                        LED_yellow_2A <= '1';
                        LED_red_2A    <= '0';
								
                    when etape6 =>
                        LED_green_1A  <= '0';
                        LED_yellow_1A <= '0';
                        LED_red_1A    <= '1';
                        LED_green_2A  <= '0';
                        LED_yellow_2A <= '0';
                        LED_red_2A    <= '1';
                end case;	 
	 end process;
	with SW_MODE select
		 LED_green_1 <= LED_green_1A when "1100",
							 LED_green_1M when "1010",
							 '0'           when others;

	with SW_MODE select
		 LED_yellow_1 <= LED_yellow_1A when "1100",
							  LED_yellow_1M when "1010",
							  LED_yellow_1S when "1001",
							  '0'            when others;

	with SW_MODE select
		 LED_red_1 <= LED_red_1A when "1100",
						  LED_red_1M when "1010",
						  '0'         when others;

	with SW_MODE select
		 LED_green_2 <= LED_green_2A when "1100",
							 LED_green_2M when "1010",
							 '0'           when others;

	with SW_MODE select
		 LED_yellow_2 <= LED_yellow_2A when "1100",
							  LED_yellow_2M when "1010",
							  LED_yellow_2S when "1001",
							  '0'            when others;

	with SW_MODE select
		 LED_red_2 <= LED_red_2A when "1100",
						  LED_red_2M when "1010",
						  '0'         when others;

end Behavioral;
