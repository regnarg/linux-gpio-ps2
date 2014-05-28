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
    char data[1024];
    int iter[1024];
    int num= 0;
    int last_clk = 1;
    for (int i = 0; i < 10000000; i++) {
        int clk = digitalRead(CLOCK_PIN);
        if (last_clk == 1 && clk == 0) {
            //printf("bit\n");
            // falling edge
            data[num] = digitalRead(DATA_PIN);
            iter[num] = i;
            num++;
            if (num > 1000) break;
        }
        last_clk = clk;
    }


    printf("wire: ");
    for (int i = 0; i< num; i++) {
        printf("%d", (int)data[i]);
    }
    printf("\n");
    for (int i = 0; i< num; i++) {
        printf("%d\n", iter[i]);
    }

    exit(0);

    int parity = 1; // odd parity
    printf("got: ");
    for (int i = 8; i >= 1; i--) {
        printf("%d", (int)data[i]);
        parity ^= data[i];
    }
    printf("\n");

    if (data[9] != parity)
        printf("Parity mismatch.\n");
    if (data[0] != 0) 
        printf("Start bit mismatch.\n");
    if (data[10] != 1)
        printf("Stop bit mismatch.\n");

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
  if ( wiringPiISR (DATA_PIN, INT_EDGE_FALLING, &myInterrupt) < 0 ) {
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

