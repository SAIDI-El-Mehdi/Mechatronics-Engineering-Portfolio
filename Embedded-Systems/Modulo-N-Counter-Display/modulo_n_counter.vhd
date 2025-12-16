library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity modulo_n_counter is
    port (
		  N : unsigned(4 downto 0);
		  clk     : in  std_logic;
        reset   : in  std_logic;
        enable  : in  std_logic;
        LEDS    : out std_logic_vector(4 downto 0);
        HEX0, HEX1 : out std_logic_vector(6 downto 0)
    );
end modulo_n_counter;

architecture Behavioral of modulo_n_counter is
    signal count : unsigned(31 downto 0) := (others => '0');
	 constant duree : unsigned(31 downto 0) := to_unsigned(50_000_0000, 32);
    signal counter_reg     : unsigned(4 downto 0) := (others => '0');
    signal count_internal  : std_logic_vector(4 downto 0); 
    signal Tens, Ones      : std_logic_vector(3 downto 0);
    signal HEX0_internal, HEX1_internal : std_logic_vector(6 downto 0);
    signal static_display : std_logic := '0';  -- Controls muxing for display

    component BinToBCD is
        Port (
            Bin         : in std_logic_vector(4 downto 0); 
            Tens, Ones  : out std_logic_vector(3 downto 0)
        );
    end component;

    component BCD_to_7Segment is
        Port (
            BCD       : in std_logic_vector(3 downto 0); 
            Segments  : out std_logic_vector(6 downto 0)
        );
    end component;

begin

    -- Convert binary count to BCD
    Converter   : BinToBCD         port map(Bin => count_internal, Tens => Tens, Ones => Ones);
    DisplayTens : BCD_to_7Segment  port map(BCD => Tens, Segments => HEX1_internal);
    DisplayOnes : BCD_to_7Segment  port map(BCD => Ones, Segments => HEX0_internal);

    -- Counter logic
    process(clk)
    begin
        if rising_edge(clk) then
            if reset = '0' then
                counter_reg <= (others => '0');
            elsif enable = '1' then
                if counter_reg = N then
                    counter_reg <= (others => '0');
                else
						 if count < duree then
						 count <= count + 1;
						 else
						 counter_reg <= counter_reg + 1;
						 count <= (others => '0');
						 end if;
                end if;
            end if;

            -- Control whether to show static display
            if N > 30 then
                static_display <= '1';
            else
                static_display <= '0';
            end if;
        end if;
    end process;

    -- Assign count to output
    count_internal <= std_logic_vector(counter_reg);

    -- Final MUX to resolve multiple driver issue
    HEX0 <= "0101111" when static_display = '1' else HEX0_internal;
    HEX1 <= "0000110" when static_display = '1' else HEX1_internal;
	 LEDS <= "00000" when static_display = '1' else count_internal;

end Behavioral;
