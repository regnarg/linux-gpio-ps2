/*
isr4pi.c
D. Thiebaut
based on isr.c from the WiringPi library, authored by Gordon Henderson
https://github.com/WiringPi/WiringPi/blob/master/examples/isr.c

Compile as follows:

    gcc -o isr4pi isr4pi.c -lwiringPi

Run as follows:

    sudo ./isr4pi

 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <time.h>
#include <unistd.h>



#define CLOCK_PIN 0 // BCM GPIO 17
#define DATA_PIN  7 // BCM GPIO 4

// the event counter 
volatile int eventCounter = 0;

// -------------------------------------------------------------------------
// myInterrupt:  called every time an event occurs
void myInterrupt(void) {
    printf("int\n");
    char data[1024];
    int num= 0;
    int last_clk = 0;
    while (1) {
        int clk = digitalRead(CLOCK_PIN);
        if (last_clk == 1 && clk == 0) {
            // falling edge
            data[num++] = digitalRead(DATA_PIN);
        }
        last_clk = clk;
        usleep(10);
        if (num >= 10) break;
    }

    printf("got: ");
    for (int i = 0; i < num; i++) printf("%d", (int)data[i]);
    printf("\n");
    //exit(0);
}


// -------------------------------------------------------------------------
// main
int main(void) {
  // sets up the wiringPi library
  if (wiringPiSetup () < 0) {
      fprintf (stderr, "Unable to setup wiringPi: %s\n", strerror (errno));
      return 1;
  }

  // set Pin 17/0 generate an interrupt on high-to-low transitions
  // and attach myInterrupt() to the interrupt
  if ( wiringPiISR (CLOCK_PIN, INT_EDGE_FALLING, &myInterrupt) < 0 ) {
      fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno));
      return 1;
  }

  // display counter value every second.
  while ( 1 ) {
    //printf( "%d\n", eventCounter );
    //eventCounter = 0;
    delay( 1000 ); // wait 1 second
  }

  return 0;
}

