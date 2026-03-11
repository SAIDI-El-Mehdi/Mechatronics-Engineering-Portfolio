#include <stdio.h>
#include "system.h"
#include "altera_avalon_pio_regs.h"
#include "unistd.h"

int main()
{
  printf("Systeme pret: Appuyez sur le Switch!\n");

  int switch_val = 0;

  while(1)
  {
      // 1. Lecture du Switch (Input)
      // PIO_0_BASE : قراءة القيمة من الساروت
      switch_val = IORD_ALTERA_AVALON_PIO_DATA(PIO_0_BASE);

      // 2. Verification de l'etat
      if (switch_val == 1)
      {
          // Allumer la LED (شعل البولة)
          IOWR_ALTERA_AVALON_PIO_DATA(LEDS_BASE, 1);
          printf("Switch ON -> LED ALLUMEE\n");
      }
      else
      {
          // Eteindre la LED (طفي البولة)
          IOWR_ALTERA_AVALON_PIO_DATA(LEDS_BASE, 0);
          printf("Switch OFF -> LED ETEINTE\n");
      }

      usleep(100000); // Attente (100ms)
  }

  return 0;
}
