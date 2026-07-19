library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity vga_blocks_top is
  port(
    CLOCK_50 : in  std_logic;

    VGA_HS   : out std_logic;
    VGA_VS   : out std_logic;
    VGA_R    : out std_logic_vector(3 downto 0);
    VGA_G    : out std_logic_vector(3 downto 0);
    VGA_B    : out std_logic_vector(3 downto 0)
  );
end entity;

architecture rtl of vga_blocks_top is

  -- 50 MHz -> 25 MHz (pixel clock)
  signal clk25 : std_logic := '0';

  -- Counters: 0..799 and 0..524
  signal h_cnt : unsigned(9 downto 0) := (others => '0');
  signal v_cnt : unsigned(9 downto 0) := (others => '0');

  signal video_on : std_logic;

  signal r, g, b : std_logic_vector(3 downto 0);

begin

  -- Clock divider
  process(CLOCK_50)
  begin
    if rising_edge(CLOCK_50) then
      clk25 <= not clk25;
    end if;
  end process;

  -- VGA timing counters
  process(clk25)
  begin
    if rising_edge(clk25) then
      if h_cnt = 799 then
        h_cnt <= (others => '0');
        if v_cnt = 524 then
          v_cnt <= (others => '0');
        else
          v_cnt <= v_cnt + 1;
        end if;
      else
        h_cnt <= h_cnt + 1;
      end if;
    end if;
  end process;

  -- HSYNC low during [656..751] (640+16 to 640+16+96-1)
  VGA_HS <= '0' when (h_cnt >= 656 and h_cnt < 752) else '1';

  -- VSYNC low during [490..491] (480+10 to 480+10+2-1)
  VGA_VS <= '0' when (v_cnt >= 490 and v_cnt < 492) else '1';

  -- Visible area (640x480)
  video_on <= '1' when (h_cnt < 640 and v_cnt < 480) else '0';

  -- Color bars (8 vertical bars, each 80 pixels wide)
  process(h_cnt, v_cnt, video_on)
    variable x   : integer;
    variable bar : integer;
  begin
    r <= (others => '0');
    g <= (others => '0');
    b <= (others => '0');

    if video_on = '1' then
      x := to_integer(h_cnt);
      bar := x / 80;  -- 0..7

      case bar is
        when 0 => r <= "1111"; g <= "0000"; b <= "0000"; -- Red
        when 1 => r <= "0000"; g <= "1111"; b <= "0000"; -- Green
        when 2 => r <= "0000"; g <= "0000"; b <= "1111"; -- Blue
        when 3 => r <= "1111"; g <= "1111"; b <= "0000"; -- Yellow
        when 4 => r <= "1111"; g <= "0000"; b <= "1111"; -- Magenta
        when 5 => r <= "0000"; g <= "1111"; b <= "1111"; -- Cyan
        when 6 => r <= "1111"; g <= "1111"; b <= "1111"; -- White
        when others =>
          r <= "0011"; g <= "0011"; b <= "0011";         -- Gray
      end case;
    end if;
  end process;

  -- Outputs (black outside visible)
  VGA_R <= r when video_on='1' else (others => '0');
  VGA_G <= g when video_on='1' else (others => '0');
  VGA_B <= b when video_on='1' else (others => '0');

end architecture;
