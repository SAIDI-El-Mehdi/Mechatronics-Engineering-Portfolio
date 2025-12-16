library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity LAB2 is
    port (
        SW     : in std_logic_vector(7 downto 0);   -- Two 4-bit numbers (SW[7:4] and SW[3:0])
        KEY    : in std_logic;                      -- Reset button (active-low)
        LED    : out std_logic_vector(4 downto 0);  -- Binary display of result
        HEX0   : out std_logic_vector(6 downto 0);  -- Units digit (7-segment display)
        HEX1   : out std_logic_vector(6 downto 0)   -- Tens digit (7-segment display)
    );
end LAB2;

architecture Adder_4bits of LAB2 is
    component add_1_bit
        port (
            x    : in std_logic;
            y    : in std_logic;
            cin  : in std_logic;
            sum  : out std_logic;
            cout : out std_logic
        );
    end component;

    signal i_carry : std_logic_vector(2 downto 0);
    signal x, y, sum : std_logic_vector(3 downto 0);
    signal cin, cout : std_logic;
    signal sum_total : std_logic_vector(4 downto 0);
    signal bcd_tens, bcd_units : std_logic_vector(3 downto 0);

    function BCD_to_7seg(BCD : std_logic_vector(3 downto 0)) return std_logic_vector is
    begin
        case BCD is
            when "0000" => return "1000000"; -- 0
            when "0001" => return "1111001"; -- 1
            when "0010" => return "0100100"; -- 2
            when "0011" => return "0110000"; -- 3
            when "0100" => return "0011001"; -- 4
            when "0101" => return "0010010"; -- 5
            when "0110" => return "0000010"; -- 6
            when "0111" => return "1111000"; -- 7
            when "1000" => return "0000000"; -- 8
            when "1001" => return "0010000"; -- 9
            when others => return "1111111"; -- Blank
        end case;
    end function;

begin
    -- Assign input switches to x and y
    x <= SW(7 downto 4);
    y <= SW(3 downto 0);
    cin <= '0'; -- Initial carry-in is 0

    -- Instantiate 1-bit adders to create 4-bit adder
    cell_1: add_1_bit port map (x(0), y(0), cin, sum(0), i_carry(0));
    cell_2: add_1_bit port map (x(1), y(1), i_carry(0), sum(1), i_carry(1));
    cell_3: add_1_bit port map (x(2), y(2), i_carry(1), sum(2), i_carry(2));
    cell_4: add_1_bit port map (x(3), y(3), i_carry(2), sum(3), cout);

    -- Combine cout and sum into 5-bit result
    sum_total <= cout & sum;
    -- Convert binary sum to BCD digits
    process(sum_total, KEY)
        variable sum_total_int : integer range 0 to 31;
    begin
        if KEY = '0' then -- Active-low reset
            bcd_tens <= "0000";
            bcd_units <= "0000";
        else
            sum_total_int := to_integer(unsigned(sum_total));
				bcd_tens <= std_logic_vector(to_unsigned(sum_total_int / 10, 4));
				bcd_units <= std_logic_vector(to_unsigned(sum_total_int mod 10, 4));
        end if;
    end process;

    -- Assign outputs with reset handling
    LED <= sum_total when KEY = '1' else "00000";
    HEX0 <= BCD_to_7seg(bcd_units) when KEY = '1' else BCD_to_7seg("0000");
    HEX1 <= BCD_to_7seg(bcd_tens) when KEY = '1' else BCD_to_7seg("0000");

end Adder_4bits;