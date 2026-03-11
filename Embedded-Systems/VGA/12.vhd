library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity VGA_Controller is
    port (
        CLOCK_50 : in  std_logic;
        VGA_HS   : out std_logic;
        VGA_VS   : out std_logic;
        VGA_R    : out std_logic_vector(3 downto 0);
        VGA_G    : out std_logic_vector(3 downto 0);
        VGA_B    : out std_logic_vector(3 downto 0)
    );
end entity VGA_Controller;

architecture rtl of VGA_Controller is

    signal h_count : integer range 0 to 799 := 0;
    signal v_count : integer range 0 to 524 := 0;
    signal clk_25  : std_logic := '0';

begin

    -- Process 1: Clock Divider (50MHz -> 25MHz)
    process(CLOCK_50)
    begin
        if rising_edge(CLOCK_50) then
            clk_25 <= not clk_25;
        end if;
    end process;

    -- Process 2: H_Count and V_Count
    process(clk_25)
    begin
        if rising_edge(clk_25) then
            if h_count = 799 then
                h_count <= 0;
                if v_count = 524 then
                    v_count <= 0;
                else
                    v_count <= v_count + 1;
                end if;
            else
                h_count <= h_count + 1;
            end if;
        end if;
    end process;

    -- Process 3: Sync Signals
    VGA_HS <= '0' when (h_count >= 656 and h_count < 752) else '1';
    VGA_VS <= '0' when (v_count >= 490 and v_count < 492) else '1';

    -- Process 4: Colors (Draw Flags/Blocks)
    process(h_count, v_count)
    begin
        if (h_count < 640 and v_count < 480) then
            -- Zone d'affichage
            if h_count < 213 then
                VGA_R <= "1111"; VGA_G <= "0000"; VGA_B <= "0000"; -- Red
            elsif h_count < 426 then
                VGA_R <= "0000"; VGA_G <= "1111"; VGA_B <= "0000"; -- Green
            else
                VGA_R <= "0000"; VGA_G <= "0000"; VGA_B <= "1111"; -- Blue
            end if;
        else
            -- Blanking (Noir)
            VGA_R <= "0000"; VGA_G <= "0000"; VGA_B <= "0000";
        end if;
    end process;

end architecture rtl;