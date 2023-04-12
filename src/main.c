// main.c
// tentaku_mod
// vika

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include "main.h"

#define STB PORTD7
#define CLK PORTD6
#define DIO PORTD5

#define INTPER 65523 // through trial and error, this one is about 32 kHz
                     // about 15 us of thinking time !

void intsetup();

// set variables modified by ISR as volatile
volatile unsigned char bytebuf = 0;
volatile int bitsleft = 8;
volatile int ready = 0;

unsigned char queue[8] = { 0x40, 0xC0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
// unsigned char dataqueue[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

int main (void) {
   intsetup();                               // setup interrupt system

   bytebuf = 0x88;                           // queue initial screen on command
   ready = 1;                                // ready to send next byte
   while(1) {
      if (ready == 0 && queue[0]) {          // get next command as long as there is one
         bytebuf = queue[0];                 // set next command to first in queue
         for (int i = 0; i < 7; i++) {  
            queue[i] = queue[i+1];           // shift queue left
         }
         queue[7] = 0;                       // clear queue end to prevent dupe
         ready = 1;                          // ready to send next byte
      }
   }
}
// ISR with TIMER1_OVF_vect
// this ISR runs when the timer counter overflows
// timer resets whenever it counts down to INTPER from 2^16 at 1/(8e6 Hz/ 8) s
ISR (TIMER1_OVF_vect) {
   if (!bitsleft && (PORTD & (1 << CLK))) {           // when done sending, reset all pins
      bitsleft = 8;
      ready = 0;
      PORTD = (1 << STB) | (1 << CLK);
   } else if (ready) {
      PORTD &= ~(1 << STB);            // set STB low as long as we are sending
      if (PORTD & (1 << CLK)) {        // if clock is high, load next LSB
         PORTD &= ~(1 << PORTD5);         // clear data pin
         PORTD |= ((bytebuf & 1) << PORTD5); // extract LSB, shift to DIO and set
         PORTD ^= (1 << CLK);             // toggle clock
         bytebuf >>= 1;                   // right shift once for next bit
         bitsleft--;                      // next bit
      } else {                         // if clock is low, set high
         PORTD ^= (1 << CLK);             // toggle clock
      }
   }
   TCNT1 = INTPER;               // 15.8 us for 8MHz clock
}

// interrupt setup code
void intsetup() {
   DDRD = (1 << DDD5) | (1 << DDD6) | (1 << DDD7); // set pins 5-7 to output
   PORTD = (1 << STB) | (1 << CLK); // set STB, CLK high
   TCNT1 = INTPER;               // 15.8 us
   TCCR1A = 0;                   // use normal counter mode
   TCCR1B = (1 << CS11);         // divide F_CPU by 1024
   TIMSK1 = (1 << TOIE1);        // trigger interrupt on counter overflow
   sei();                        // set global interrupt flag
}
