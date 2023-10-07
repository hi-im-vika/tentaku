#include "spi.h"

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

// reset CLK and STB states
void reset() {
   PORTD |= ((1 << CLK) | (1 << STB)); // set CLK and STB
}
