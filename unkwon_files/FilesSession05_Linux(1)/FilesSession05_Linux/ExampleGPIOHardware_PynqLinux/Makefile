all: testGPIO

testGPIO: testGPIO.cpp OverlayControl.c OverlayControl.h
	g++ -O3 -Wall testGPIO.cpp OverlayControl.c -o testGPIO -lm 

clean:
	rm -f testGPIO

