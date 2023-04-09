// main.c
// tentaku_mod
// gotta use bitwise ops to do bit manupulation
// who woulda guessed
// PORTD5: DIO
// PORTD6: CLK
// PORTD7: STB
// vika

#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include "main.h"

#define STB PORTD7
#define CLK PORTD6
#define DIO PORTD5
#define DTIME 5

void reset();
const unsigned char zeroes[8] = { 0 };
void sendbyte(unsigned char);
void readbytes(unsigned char*);
void sendbuf(unsigned char*);
void parse(unsigned char*, unsigned char*, int*);
int main (void) {
   unsigned char buf[8] = { 0x00, 0x5B, 0x4F, 0x66, 0x77, 0x7C, 0x58, 0x5E, };
   unsigned char readbuf[8] = { 0 };
   int next = 1;
   while(1) {
      DDRD = 0b11100000;
      reset();
      readbytes(readbuf);
      parse(readbuf, buf, &next);
      sendbyte(0x88);
      sendbuf(buf);
   }

   //  scrolling code
   //
   //   while(1) {
   //      unsigned char readbuf = 0;
   //      for (int a = 0; a < 8; a++) {
   //         unsigned char pushed = buf[0];
   //         for (int x = 0; x < 8; x++) {
   //            buf[x] = buf[x+1];
   //         }
   //         buf[7] = pushed;
   //         _delay_ms(100);
   //         sendbuf(buf);
   //      }
   //   }
}

// take arrays rb and b and int n, look at what rb is and
// change b accordingly
void parse(unsigned char *rb, unsigned char *b, int *n) {
   if (rb[0] == 0b00000100) {            // 0 pressed
      b[0] = chars[0];
   } else if (rb[2] == 0b00100000) {     // 1 pressed
      b[0] = chars[1];
   } else if (rb[3] == 0b00000010) {     // 2 pressed
      b[0] = chars[2];
   } else if (rb[3] == 0b00100000) {     // 3 pressed
      b[0] = chars[3];
   } else if (rb[1] == 0b00000010) {     // 4 pressed
      b[0] = chars[4];
   } else if (rb[1] == 0b00100000) {     // 5 pressed
      b[0] = chars[5];
   } else if (rb[2] == 0b00000010) {     // 6 pressed
      b[0] = chars[6];
   } else if (rb[3] == 0b01000000) {     // 7 pressed
      b[0] = chars[7];
   } else if (rb[0] == 0b00000010) {     // 8 pressed
      b[0] = chars[8];
   } else if (rb[0] == 0b00100000) {     // 9 pressed
      b[0] = chars[9];
   } else if (rb[0] == 0b00000001) {     // . pressed
      b[0] = 0b10000000;
   } else if (rb[1] == 0b00000100) {     // ENTER pressed
      if ((*n)++ > 8) {
         *n = 1;
      }
      b[*n] = b[0];
   } else if (rb[1] == 0b01000000) {     // NUMLOCK pressed
      for (int i = 0; i < 8; i++) {
         b[i] = 0;
      }
   } else {
      b[0] = b[0];
   }
}

// shifts out cmd (MSB first), where nbits is length of cmd
// without affecting STB
void shiftout(int nbits, unsigned char cmd) {
   for (int cbit = 0; cbit < nbits; cbit++) {
      PORTD ^= (1 << CLK); // toggle CLK (low)
      PORTD &= ~(1 << DIO); // reset DIO
      PORTD |= (((cmd & (1<< cbit)) >> cbit) << DIO);
            // create 8 bit mask with lsb set, shift that bit left "cbit" times
            // AND with cmd to get the bit of interest
            // shift extracted bit back to LSB to "reset" position
            // shift bit left to line up with DIO
      _delay_us(DTIME);
      PORTD ^= (1 << CLK); // toggle CLK (high)
      _delay_us(DTIME);
   }
}

void readbytes(unsigned char *readbuf) {
   unsigned char oldstate = DDRD;
   DDRD = 0b11100000; // set PORTD5 to PORTD7 as output
   PORTD &= ~(1 << STB); // bring STB low
   _delay_us(DTIME);
   shiftout(8, 0x42);
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

void sendbyte(unsigned char b) {
   PORTD &= ~(1 << STB); // bring STB low
   shiftout(8, b);
   PORTD |= (1 << STB); // bring STB high
}

void sendbuf(unsigned char *buf) {
   sendbyte(0x40); // set auto increment mode
   PORTD &= ~(1<<STB); // bring STB low
   shiftout(8, 0xC0);
   for (int reg = 0; reg < 8; reg++) {
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
         shiftout(8, 0);
      }
   }
   PORTD |= (1<<STB); // bring STB high
}

void reset() {
   PORTD |= ((1<<CLK) | (1<<STB)); // bring CLK and STB high
}
