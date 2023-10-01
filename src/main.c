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

void reset();
const uint8_t zeroes[8] = { 0 };
void sendByte(uint8_t);
void readbytes(uint8_t*);
void sendBuffer(uint8_t*);
void readBytes(uint32_t*);
void parse(uint32_t, uint8_t*, int*);
void intSetup();

uint8_t tog = (1 << 7);

int main (void) {
   uint8_t buf[8] = { SEG_1, SEG_2, SEG_3, SEG_4, SEG_A, SEG_B, SEG_C, SEG_D };
   uint32_t keyStates = 0;
   int next = 1;
   intSetup();
   while(1) {
      DDRD = SPIALL;
      reset();
      readBytes(&keyStates);
      parse(keyStates, buf, &next);

      buf[0] = (buf[0] & (((uint8_t) -1) >> 1)) | tog;
      // blink based on interrupt clock cycle

      sendByte(0x88);
      sendBuffer(buf);
   }

   //  scrolling code
   //
   //   while(1) {
   //      uint8_t readbuf = 0;
   //      for (int a = 0; a < 8; a++) {
   //         uint8_t pushed = buf[0];
   //         for (int x = 0; x < 8; x++) {
   //            buf[x] = buf[x+1];
   //         }
   //         buf[7] = pushed;
   //         _delay_ms(100);
   //         sendBuffer(buf);
   //      }
   //   }
}

// ISR with TIMER1_OVF_vect
// this ISR runs when the timer counter overflows
// the timer overflows whenever it counts down to INTPER from 2^16
ISR (TIMER1_OVF_vect) {
   tog ^= (1 << 7);
   TCNT1 = INTPER; // 15.8 us for 8MHz clock
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
void parse(uint32_t keys, uint8_t *b, int *n) {
   switch (keys) {
      case NP_ENT:
         if ((*n)++ > 8) {
            *n = 1;
         }
         b[*n] = b[7];
         break;
      case NP_NUM:
         for (int i = 0; i < 8; i++) {
            b[i] = 0;
         }
         break;
      default:
         b[7] = b[7];
         break;
   }
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

void readbytes(uint8_t *readbuf) {
   uint8_t oldstate = DDRD;
   DDRD = 0b11100000; // set PORTD5 to PORTD7 as output
   PORTD &= ~(1 << STB); // bring STB low
   _delay_us(DTIME);
   shiftOut(8, 0x42);
   PORTD &= ~(1 << DIO); // reset DIO
   DDRD = 0b11000000;
   for (int cyc = 0; cyc < 4; cyc++) {
      for (int d = 0; d < 8; d++) {
         PORTD ^= (1 << CLK); // toggle CLK (low)
         _delay_us(DTIME);
         PORTD ^= (1 << CLK); // toggle CLK (high)
         readbuf[cyc] &= ~(((-1) >> 7) << d);
         readbuf[cyc] |= (((PIND & (1 << PIND5)) >> PIND5) << d); //|= masked;
         _delay_us(DTIME);
      }
   }
   PORTD |= (1 << STB); // bring STB high
   _delay_us(DTIME);
   DDRD = oldstate;
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
