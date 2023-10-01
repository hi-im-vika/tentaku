// main.c
// tentaku_mod
// gotta use bitwise ops to do bit manupulation
// who woulda guessed
// PORTD5: DIO (VIO2)
// PORTD6: CLK (GRN2)
// PORTD7: STB (PCH2)
// SPI: PCH GRN VIO BRN BLK RED
// vika

#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include "main.h"

#define STB PORTD7
#define CLK PORTD6
#define DIO PORTD5
#define SPIALL ((1 << STB) | (1 << CLK) | (1 << DIO))
#define DTIME 5

// #define INTPER 65523 // through trial and error, this one is about 32 kHz
// about 15 us of thinking time !
#define INTPER 1

// function prototypes
void setup();
void intSetup();
void sendByte(uint8_t);
void sendBuffer(uint8_t*);
void readBytes(uint32_t*);
void parseInput(uint32_t, uint8_t*, int*);
int parseDisplay(uint8_t*);
void reset();

// global variables
volatile uint8_t blink = (1 << 7);
const uint8_t zeroes[8] = { 0 };
int stackA = 0;
int stackB = 0;

int main (void) {
   // uint8_t buf[8] = { SEG_1, SEG_2, SEG_3, SEG_4, SEG_A, SEG_B, SEG_C, SEG_D };
   uint8_t buf[8] = { 0 };
   uint32_t keyStates = 0;
   int next = 1;
   uint32_t prevKeys = 0;
   setup();
   intSetup();
   while(1) {
      sendByte(0x33); // meaningless data for trigger on logic analyzer
      prevKeys = keyStates;
      readBytes(&keyStates);
      if ((prevKeys != keyStates) && (prevKeys == 0x00)) {
         parseInput(keyStates, buf, &next);
      }

      // blink based on interrupt clock cycle
      buf[0] = (buf[0] & ~(0x80)) | blink;

      sendByte(0x88);
      sendBuffer(buf);
   }
}

// ISR with TIMER1_OVF_vect
// this ISR runs when the timer counter overflows
// the timer overflows whenever it counts down to INTPER from 2^16
ISR (TIMER1_OVF_vect) {
   blink ^= (1 << 7);
   TCNT1 = INTPER; // 15.8 us for 8MHz clock
}

// general setup code
void setup() {
   DDRD = SPIALL;
   reset();
}

// interrupt setup code
void intSetup() {
   TCNT1 = INTPER;         // counter starts at overflow value
   TCCR1A = 0x00;          // counter normal mode
   TCCR1B = (1 << CS11);   // enable pre-scaler (F_CPU/8)
   TIMSK1 = (1 << TOIE1);  // enable interrupt trigger on overflow
   sei();                  // global interrupt enable
}

// take arrays rb and b and int n, look at what rb is and
// change b accordingly
void parseInput(uint32_t keys, uint8_t *b, int *n) {
   int nextSeg = 0;
   int isNumber = 0;
   switch (keys) {
      case NP_0:
         isNumber = 1;
         nextSeg = SEG_0;
         break;
      case NP_1:
         isNumber = 1;
         nextSeg = SEG_1;
         break;
      case NP_2:
         isNumber = 1;
         nextSeg = SEG_2;
         break;
      case NP_3:
         isNumber = 1;
         nextSeg = SEG_3;
         break;
      case NP_4:
         isNumber = 1;
         nextSeg = SEG_4;
         break;
      case NP_5:
         isNumber = 1;
         nextSeg = SEG_5;
         break;
      case NP_6:
         isNumber = 1;
         nextSeg = SEG_6;
         break;
      case NP_7:
         isNumber = 1;
         nextSeg = SEG_7;
         break;
      case NP_8:
         isNumber = 1;
         nextSeg = SEG_8;
         break;
      case NP_9:
         isNumber = 1;
         nextSeg = SEG_9;
         break;
      case NP_ENT:
         for (int x = 0; x < 8; x++) {
            b[x] = b[7];
         }
         break;
      case NP_NUM:
         for (int i = 0; i < 8; i++) {
            b[i] = 0;
         }
         break;
      case NP_DEC:
         b[7] |= SEGPART_7;
         break;
      default:
         b[7] = b[7];
         break;
   }
   if (isNumber) {
      for (int n = 0; n < 7; n++) {
         b[n] = b[n+1];
      }
      b[7] = nextSeg;
   }
}

int parseDisplay(uint8_t *buf) {
   return 0;
}

// shifts out cmd (LSB first), where bits is length of cmd (in bits)
// does not affect STB
// this is OK
void shiftOut(int bits, uint8_t cmd) {
   DDRD |= (1 << DIO);                       // configure DIO as output
   for (int bit = 0; bit < bits; bit++) {
      PORTD &= ~((1 << DIO) | (1 << CLK));   // clear CLK and DIO

      // mask out relevant bit, reduce to 0 or 1 with !!, shift bit to line up with DIO
      PORTD |= (!!(cmd & (1 << bit)) << DIO);

      _delay_us(DTIME);
      PORTD |= (1 << CLK);                   // set CLK (high), clock out the bit
      _delay_us(DTIME);
   }
}

void readBytes(uint32_t *keys) {
   *keys = 0;              // clear last read key state
   DDRD = SPIALL;          // set all SPI pins to output
   PORTD &= ~(1 << STB);   // clear STB (begin transmission)
   _delay_us(DTIME);
   shiftOut(8, 0x42);      // send read command
   PORTD &= ~(1 << DIO);   // clear DIO
   DDRD &= ~(1 << DIO);    // set DIO to input
                           // read in 32 bits
   for (int bit = 0; bit < 32; bit++) {
      PORTD &= ~(1 << CLK);   // clear CLK
      _delay_us(DTIME);
      PORTD |= (1 << CLK);    // set CLK (read single bit)
      *keys |= (((uint32_t) !!(PIND & (1 << DIO))) << bit); // check DIO and set corresponding bit in integer
      _delay_us(DTIME);
   }
   PORTD |= (1 << STB);    // set STB (end transmission)
   _delay_us(DTIME);
}

// send a command byte
void sendByte(uint8_t b) {
   PORTD &= ~(1 << STB); // bring STB low
   shiftOut(8, b);
   PORTD |= (1 << STB); // bring STB high
}

// send buffer array consisting of characters to display
void sendBuffer(uint8_t *buf) {
   sendByte(0x40);               // send command to set auto increment mode
   PORTD &= ~(1 << STB);         // clear STB (begin conversation)
   shiftOut(8, 0xC0);            // send command to set start address at 0x00

   // iterate through buffer
   for (int bit = 0; bit < 8; bit++) {

      // send info for entire character
      for (int chr = 0; chr < 8; chr++) {
         PORTD &= ~(1 << CLK);   // clear CLK
         _delay_us(DTIME);
         PORTD &= ~(1 << DIO);   // clear DIO, prepare for data output

         // mask out relevant bit, reduce to 0 or 1 with !!, shift bit to line up with DIO
         PORTD |= (!!(buf[chr] & (1 << bit)) << DIO);

         PORTD ^= (1 << CLK);    // set CLK (clock out bit)
         _delay_us(DTIME);
      }

      // fill every second register with zeroes
      // (every second register is unused)
      shiftOut(8, 0);
   }
   PORTD |= (1 << STB);          // set STB (end conversation)
}

// reset CLK and STB states
void reset() {
   PORTD |= ((1 << CLK) | (1 << STB)); // set CLK and STB
}
