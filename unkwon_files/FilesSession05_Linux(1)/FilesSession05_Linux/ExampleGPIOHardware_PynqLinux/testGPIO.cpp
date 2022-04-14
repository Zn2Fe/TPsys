#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include "OverlayControl.h"

// Base address for the mapping of (all) peripherals
#define BASE_MAP  0x40000000
// Offset of the first GPIO device (GPIO = BASE_MAP + GPIO1_OFFSET) - Buttons
#define BUTTONS_OFFSET 0
// Offset of the second GPIO device (GPIO = BASE_MAP + GPIO2_OFFSET) - LEDs
#define LEDS_OFFSET 0x3000000

// Adjust the size of the mapping to cover all the peripherals, or use multiple mappings.
const uint32_t MAP_SIZE = 64*1024*1024; // 0x400_0000

enum {GPIO_TRISTATE = 1}; // Offsets are in 32-bit words!

// Bits by weight
#define TESTBIT(x, y) (((x) & (y)) ? 1 : 0)

int main(int argc, char ** argv)
{
  volatile uint8_t * device = NULL;
  volatile uint32_t * BUTTONS;
  volatile uint32_t * LEDS;

  printf("\n\nThis program requires that the GPIO buttons and LEDs bitstream is loaded in the FPGA.\n");
  printf("This program has to be run with sudo.\n");
  printf("Press ENTER to confirm that the bitstream is loaded (proceeding without it can crash the board).\n\n");
  getchar();

  // Obtain a pointer to access the peripherals in the address map.
	device = (uint8_t*) MapMemIO(BASE_MAP, MAP_SIZE);
	if (device == NULL) {
		printf("Error opening device!\n");
    exit(-1);
	}
  printf("Mmap done. Peripherals at %08X\n", (uint32_t)device);

  BUTTONS = (uint32_t*)(device + BUTTONS_OFFSET);
  LEDS = (uint32_t*)(device + LEDS_OFFSET);
  printf("Buttons at: %08X - LEDs at: %08X\n", (uint32_t)BUTTONS, (uint32_t)LEDS);

  // Init GPIOs for reading / writing.
  *(BUTTONS + GPIO_TRISTATE) = 0xFFFFFFFF;
  *(LEDS + GPIO_TRISTATE) = 0xFFFFFFF0;
  printf("Tristates done!\n\n");

  uint32_t count = 0;
  while (1) {
    uint32_t buttons;

    buttons = *BUTTONS;
    *LEDS = buttons;

    ++ count;
    if ( (count % 50) == 0 ) {  // Limit to twice per second
      printf("Button status: %.1u%.1u%.1u%.1u - %u repetitions          \r", 
        TESTBIT(buttons, 8), TESTBIT(buttons, 4), TESTBIT(buttons, 2), TESTBIT(buttons, 1),
        count);
      fflush(stdout); // To force writing to the terminal (necesary with \r).
    }
    usleep(10000);
  }

  //////////////////////////////////////////////  

	UnmapMemIO();

	return 0;
}

