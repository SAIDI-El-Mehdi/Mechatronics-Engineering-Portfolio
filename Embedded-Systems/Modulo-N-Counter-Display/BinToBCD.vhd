------------------------------------------------------------
-- Binary to BCD Converter
------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity BinToBCD is
    Port (
        Bin : in STD_LOGIC_VECTOR(4 downto 0); -- 5-bit binary input
        Tens, Ones : out STD_LOGIC_VECTOR(3 downto 0) -- BCD outputs
    );
end BinToBCD;

architecture Behavioral of BinToBCD is
begin
    process(Bin)
    variable Temp : INTEGER range 0 to 31;
    variable T, O : INTEGER range 0 to 9;
    begin
        Temp := CONV_INTEGER(Bin);
        T := Temp / 10;
        O := Temp mod 10;
        Tens <= CONV_STD_LOGIC_VECTOR(T, 4);
        Ones <= CONV_STD_LOGIC_VECTOR(O, 4);
    end process;
end Behavioral;