#include <stdio.h>
#include <stdint.h>
#include "platform.h"
#include <sleep.h>

#define PWM_MAX 1000000
#define PERIOD_INCREMENT  5000

// Bits by weight
#define TESTBIT(x, y) (((x) & (y)) ? 1 : 0)

// GPIO 0
volatile uint32_t * BUTTONS = (volatile uint32_t*)0x41200000;
volatile uint32_t * SWITCHES = (volatile uint32_t*)0x41200008;
enum {GPIO_TRISTATE = 1}; // Offsets are in 32-bit words!

volatile uint32_t * RGB0 = (volatile uint32_t *)0x43C00000;
volatile uint32_t * RGB1 = (volatile uint32_t *)0x43C10000;
enum {PWM_PERIOD = 0, ENABLE = 1, BLUE = 2, GREEN = 3, RED = 4}; // Offsets are in 32-bit words!

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
  int res = TRUE;

  value = *counter;

  if (((increment >= 0) && ((maxValue - value) >= increment)) || 
      ((increment < 0) && (value >= -increment)))
  {
    * counter = value + increment;
    res = FALSE;
  }

  return res;
}

int main()
{
  uint32_t count = 0;
  init_platform();

  printf("Hello world!\n");

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
    hitLimit = FALSE;
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
      PrintRegs(RGB0, FALSE);
      PrintRegs(RGB1, FALSE);
      printf("          \r");	// Print extra spaces before carriage return to remove garbage when the numbers decrease in length.
      fflush(stdout);	// Without \n, the buffer of stdout is not flushed after the printing.
    }
    usleep(50000);
  }

  cleanup_platform();
  return 0;
}
