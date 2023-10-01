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
void sendbuf(uint8_t*);
void parse(uint8_t*, uint8_t*, int*);
void intSetup();

uint8_t tog = (1 << 7);

int main (void) {
   uint8_t buf[8] = { chars[1], chars[2], chars[3], chars[4], chars[10], chars[11], chars[12], chars[13], };
   uint8_t readbuf[4] = { 0 };
   uint8_t readbuft[4] = { 0 };
   int next = 1;
   intSetup();
   while(1) {
      DDRD = SPIALL;
      reset();
      readbytes(readbuf);
      readbytes(readbuft);
      parse(readbuf, buf, &next);

      buf[0] = (buf[0] & (((uint8_t) -1) >> 1)) | tog;
      // blink based on interrupt clock cycle
      
      sendByte(0x88);
      sendbuf(buf);
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
   //         sendbuf(buf);
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
   TCNT1 = INTPER; // 15.8 us for 8MHz clock
   TCCR1A = 0x00; // Set normal counter mode
   TCCR1B = (1 << CS11); // Set 8 pre-scaler
   TIMSK1 = (1 << TOIE1); // Set overflow interrupt enable bit
   sei(); // Enable interrupts globally
}

// take arrays rb and b and int n, look at what rb is and
// change b accordingly
void parse(uint8_t *readbuf, uint8_t *b, int *n) {
   if (readbuf[0] == 0b00000100) {            // 0 pressed
      b[7] = chars[0];
   } else if (readbuf[2] == 0b00100000) {     // 1 pressed
      b[7] = chars[1];
   } else if (readbuf[3] == 0b00000010) {     // 2 pressed
      b[7] = chars[2];
   } else if (readbuf[3] == 0b00100000) {     // 3 pressed
      b[7] = chars[3];
   } else if (readbuf[1] == 0b00000010) {     // 4 pressed
      b[7] = chars[4];
   } else if (readbuf[1] == 0b00100000) {     // 5 pressed
      b[7] = chars[5];
   } else if (readbuf[2] == 0b00000010) {     // 6 pressed
      b[7] = chars[6];
   } else if (readbuf[3] == 0b01000000) {     // 7 pressed
      b[7] = chars[7];
   } else if (readbuf[0] == 0b00000010) {     // 8 pressed
      b[7] = chars[8];
   } else if (readbuf[0] == 0b00100000) {     // 9 pressed
      b[7] = chars[9];
   } else if (readbuf[0] == 0b00000001) {     // . pressed
      b[7] = 0b10000000;
   } else if (readbuf[1] == 0b00000100) {     // ENTER pressed
      if ((*n)++ > 8) {
         *n = 1;
      }
      b[*n] = b[7];
   } else if (readbuf[1] == 0b01000000) {     // NUMLOCK pressed
      for (int i = 0; i < 8; i++) {
         b[i] = 0;
      }
   } else {
      b[7] = b[7];
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

void sendByte(uint8_t b) {
   PORTD &= ~(1 << STB); // bring STB low
   shiftOut(8, b);
   PORTD |= (1 << STB); // bring STB high
}

void sendbuf(uint8_t *buf) {
   sendByte(0x40); // set auto increment mode
   PORTD &= ~(1<<STB); // bring STB low
   shiftOut(8, 0xC0);
   for (int bit = 0; bit < 8; bit++) {
      // send data to register
      for (int digit = 0; digit < 8; digit++) {
         PORTD ^= (1<<CLK); // toggle CLK (low)
         _delay_us(DTIME);
         PORTD &= ~(1<<DIO); // reset DIO
         PORTD |= (((buf[digit] & (1<<bit)) >> bit)<<DIO);
         // create 8 bit mask with lsb set, shift that bit left "bit" times
         // AND with the current digit to get the bit of interest
         // shift extracted bit back to LSB to "reset" position
         // shift bit left to line up with DIO
         PORTD ^= (1<<CLK); // toggle CLK (high)
         _delay_us(DTIME);
      }
      // fill up every other register with zeroes
      shiftOut(8, 0);
   }
   PORTD |= (1<<STB); // bring STB high
}

void reset() {
   PORTD |= ((1<<CLK) | (1<<STB)); // bring CLK and STB high
}
