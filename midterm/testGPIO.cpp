#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include "OverlayControl.h"


// Base address for the mapping of (all) peripherals
#define BASE_MAP 0x43c00000
const uint32_t MAP_SIZE = 64*1024; // 0x400_0000

enum {START = 0, DONE = 1, VECTOR_START = 2, VECTOR_LENGTH = 3, SUM = 4}; // Offsets are in 32-bit words!

// Bits by weight
//#define TESTBIT(x, y) (((x) & (y)) ? 1 : 0)

int main()
{
  uint32_t count = 0;
  

  volatile uint8_t * ACCEL = NULL;

  printf("\n\nThis program requires that the GPIO buttons and LEDs bitstream is loaded in the FPGA.\n");
  printf("This program has to be run with sudo.\n");
  printf("Press ENTER to confirm that the bitstream is loaded (proceeding without it can crash the board).\n\n");
  getchar();

  // Obtain a pointer to access the peripherals in the address map.
	ACCEL = (uint8_t*) MapMemIO(BASE_MAP, MAP_SIZE);
	if (ACCEL == NULL) {
		printf("Error opening device!\n");
    exit(-1);
	}
  printf("Mmap done. Peripherals at %08X\n", (uint32_t)ACCEL);
  

  // Init GPIOs for reading.
  *(ACCEL + DONE) = 0;
  *(ACCEL + VECTOR_START) = 0;
  *(ACCEL + VECTOR_LENGTH) = 0;
  *(ACCEL + SUM) = 0;


  while (1) {
    uint32_t done, sum;
    uint32_t start, vector_length;
    u_int32_t vector_start;

    //output
    done = *(ACCEL+DONE);
    sum = *(ACCEL+SUM);
    //input 
    vector_start = *(ACCEL+VECTOR_START);
    vector_length = *(ACCEL + VECTOR_LENGTH);

    /*Printing*/
    // Reduce the frequency of messages sent thru the UART --> once per 10 second.
    ++ count;
    if ((count % 200) == 0) {
      printf("Times: %lu ", count);
      printf("The value of the registers are : \n");
      printf(" done : %lu \n",done);
      printf(" sum : %lu \n",sum);
      printf(" v_start : %lu \n",vector_start);
      printf(" v_length : %lu \n",vector_length);
    }

    usleep(50000);
  }
	UnmapMemIO();
  return 0;
}
