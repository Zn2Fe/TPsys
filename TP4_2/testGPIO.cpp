#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include "OverlayControl.h"

#define PWM_MAX 1000000
#define PERIOD_INCREMENT  5000

// Base address for the mapping of (all) peripherals
#define BASE_MAP_GPIO  0x41200000
#define BUTTONS_OFFSET 0
#define SWITCHES_OFFSET 0x8
const uint32_t MAP_SIZE_GPIO = 64*1024; // 0x400_0000
enum {GPIO_TRISTATE = 1}; // Offsets are in 32-bit words!

#define BASE_MAP_PWM  0x43c00000
#define PWM1_OFFSET 0
#define PWM2_OFFSET 0x10000
const uint32_t MAP_SIZE_PWM = 2*64*1024; // 0x400_0000
enum {PWM_PERIOD = 0, ENABLE = 1, BLUE = 2, GREEN = 3, RED = 4}; // Offsets are in 32-bit words!

// Bits by weight
#define TESTBIT(x, y) (((x) & (y)) ? 1 : 0)

void PrintRegs(volatile uint32_t * device, int newLine)
{
	printf("PWM: %lu Enable: %lu B: %lu G: %lu R: %lu %s",
			*(device + PWM_PERIOD), *(device + ENABLE),
			*(device + BLUE), *(device + GREEN), *(device + RED),
			newLine ? "\n" : "");
}

// Return 0 if success, 1 if the limit was hit.
int ReprogramCounter(volatile uint32_t * counter, int32_t increment, uint32_t maxValue)
{
  uint32_t value;
  int res = 0;

  value = *counter;

  if (((increment >= 0) && ((maxValue - value) >= increment)) || 
      ((increment < 0) && (value >= -increment)))
  {
    * counter = value + increment;
    res = 1;
  }

  return res;
}

int main()
{
  uint32_t count = 0;
  

  volatile uint8_t * gpio = NULL;
  volatile uint32_t * BUTTONS;
  volatile uint32_t * SWITCHES;

  volatile uint8_t * pwm = NULL;
  volatile uint32_t * RGB0;
  volatile uint32_t * RGB1;

  printf("\n\nThis program requires that the GPIO buttons and LEDs bitstream is loaded in the FPGA.\n");
  printf("This program has to be run with sudo.\n");
  printf("Press ENTER to confirm that the bitstream is loaded (proceeding without it can crash the board).\n\n");
  getchar();

  // Obtain a pointer to access the peripherals in the address map.
	gpio = (uint8_t*) MapMemIO(BASE_MAP_GPIO, MAP_SIZE_GPIO);
	if (gpio == NULL) {
		printf("Error opening device!\n");
    exit(-1);
	}
  printf("Mmap done. Peripherals at %08X\n", (uint32_t)gpio);

  BUTTONS = (uint32_t*)(gpio + BUTTONS_OFFSET);
  SWITCHES = (uint32_t*)(gpio + SWITCHES_OFFSET);

  // Obtain a pointer to access the peripherals in the address map.
	pwm = (uint8_t*) MapMemIO(BASE_MAP_PWM, MAP_SIZE_PWM);
	if (pwm == NULL) {
		printf("Error opening device!\n");
    exit(-1);
	}
  printf("Mmap done. Peripherals at %08X\n", (uint32_t)pwm);

  RGB0 = (uint32_t*)(pwm + PWM1_OFFSET);
  RGB1 = (uint32_t*)(pwm + PWM2_OFFSET);
  

  // Init GPIOs for reading.
  *(BUTTONS + GPIO_TRISTATE) = 0xFFFFFFFF;
  *(SWITCHES + GPIO_TRISTATE) = 0xFFFFFFFF;

  *(RGB0 + PWM_PERIOD) = PWM_MAX;
  *(RGB1 + PWM_PERIOD) = PWM_MAX;

  *(RGB0 + RED) = 0;
  *(RGB0 + GREEN) = 0;
  *(RGB0 + BLUE) = 0;
  *(RGB1 + RED) = 0;
  *(RGB1 + GREEN) = 0;
  *(RGB1 + BLUE) = 0;

  *(RGB0 + ENABLE) = 1;
  *(RGB1 + ENABLE) = 1;

  while (1) {
    uint32_t switches, buttons;
    int32_t increment;
    int setR, setG, setB; // Need to modify each led?
    int hitLimit;

    buttons = *BUTTONS;
    switches = *SWITCHES;

    increment = TESTBIT(buttons, 8) ? -PERIOD_INCREMENT : PERIOD_INCREMENT;
    setR = TESTBIT(buttons, 4);
    setG = TESTBIT(buttons, 2);
    setB = TESTBIT(buttons, 1);

    // Check action on RGB 0
    hitLimit = 1;
    if (switches & 1) {
      if (setR)
        hitLimit = hitLimit | ReprogramCounter(RGB0 + RED, increment, PWM_MAX);
      if (setG)
        hitLimit = hitLimit | ReprogramCounter(RGB0 + GREEN, increment, PWM_MAX);
      if (setB)
        hitLimit = hitLimit | ReprogramCounter(RGB0 + BLUE, increment, PWM_MAX);
    }
      // Check action on RGB 1
    if (switches & 2) {
      if (setR)
        hitLimit = hitLimit | ReprogramCounter(RGB1 + RED, increment, PWM_MAX);
      if (setG)
        hitLimit = hitLimit | ReprogramCounter(RGB1 + GREEN, increment, PWM_MAX);
      if (setB)
        hitLimit = hitLimit | ReprogramCounter(RGB1 + BLUE, increment, PWM_MAX);
    }

    // Reduce the frequency of messages sent thru the UART --> once per second.
    ++ count;
    if ((count % 20) == 0) {
      printf("Times: %lu ", count);
      //if (hitLimit)
      //  printf(" - Limit hit!\n");
      PrintRegs(RGB0, 1);
      PrintRegs(RGB1, 1);
      printf("          \r");	// Print extra spaces before carriage return to remove garbage when the numbers decrease in length.
      fflush(stdout);	// Without \n, the buffer of stdout is not flushed after the printing.
    }
    usleep(50000);
  }
	UnmapMemIO();
  return 0;
}
