#include "buffer.h"

// takes a number i and puts it onto the buffer
void putToBuffer(uint8_t *buf, int64_t i) {
   if (i < 0) {
      buf[0] |= SEGPART_6;
      i *= -1;
   }
   if (i > 9999999) {
      buf[3] = 0b01011100;
      buf[4] = 0b00111000;
      return;
   }
   for (int seg = 7; seg >= 1; seg--) {
      buf[seg] = numToSeg(numberAtPos(i,7 - seg));
   }

   // go backwards through buffer and replace leading zeroes with blank
   // exclude first zero
   for (int seg = 1; seg < 7; seg++) {
      if (buf[seg] == SEG_0) {
         buf[seg] = 0x00;
      } else {
         break;
      }
   }
}

// clear display buffer
void clearBuffer(uint8_t *buf) {
   // fill all elements of buffer with zero
   for (int seg = 0; seg < 8; seg++) {
      buf[seg] = 0;
   }
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
