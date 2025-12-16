library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;
use IEEE.NUMERIC_STD.ALL;


entity TB_LAB2 is
end TB_LAB2;

architecture Test of TB_LAB2 is

    -- Déclaration de l'entité LAB2 à tester
    component LAB2
        port (
            SW     : in std_logic_vector(7 downto 0);   -- Deux nombres de 4 bits (SW[7:4] et SW[3:0])
            KEY    : in STD_LOGIC;                       -- Bouton de reset
            LED    : out STD_LOGIC_VECTOR (4 downto 0);  -- Affichage binaire du résultat
            HEX0   : out STD_LOGIC_VECTOR (6 downto 0);  -- Chiffre des unités (afficheur 7 segments)
            HEX1   : out STD_LOGIC_VECTOR (6 downto 0)   -- Chiffre des dizaines (afficheur 7 segments)
        );
    end component;

    -- Signaux internes pour la simulation
    signal SW_TEST     : std_logic_vector(7 downto 0) := (others => '0');
    signal KEY_TEST    : std_logic := '1';  -- Initiellement non appuyé
    signal LED_TEST    : std_logic_vector(4 downto 0);
    signal HEX0_TEST   : std_logic_vector(6 downto 0);
    signal HEX1_TEST   : std_logic_vector(6 downto 0);
	 signal A 			  : std_logic_vector(3 downto 0);
	 signal B 			  : std_logic_vector(3 downto 0);

begin

    -- Instanciation de l'UUT (Unité sous Test)
    UUT: LAB2
        port map (
            SW    => SW_TEST,
            KEY   => KEY_TEST,
            LED   => LED_TEST,
            HEX0  => HEX0_TEST,
            HEX1  => HEX1_TEST
        );
	 A <= SW_TEST(7 downto 4);
	 B <= SW_TEST(3 downto 0);
    -- Processus pour tester toutes les combinaisons de SW[7:0]
    process
    begin
        -- Tester toutes les combinaisons de SW de 0 à 255
        for i in 0 to 255 loop
            SW_TEST <= std_logic_vector(to_unsigned(i, 8));  -- Assignation de la combinaison à SW_TEST
            wait for 1 ns;  -- Attendre pour observer les résultats
				if i = 30 then
					KEY_TEST <= '0';  -- Activer reset
				   wait for 10 ns;
				   KEY_TEST <= '1';  -- Désactiver reset
				   wait for 10 ns;
				end if;
        end loop;

        -- Fin de la simulation
        wait;
    end process;

end Test;
