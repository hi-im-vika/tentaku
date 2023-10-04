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
#include <string.h>
#include <math.h>       // duh lol
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
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
void reset();
void sendByte(uint8_t);
void sendBuffer(uint8_t*);
void readBytes(uint32_t*);
void parseInput(uint32_t, uint8_t*);
int segToNum(uint8_t);
uint8_t numToSeg(int);
int64_t parseDisplay(uint8_t*);
void putToBuffer(uint8_t*, int64_t);
void clearBuffer(uint8_t*);

// stack manipulation
void stackInsert(uint8_t*);

// calculator functions
void calcFuncAdd(uint8_t*);
void calcFuncSub(uint8_t*);
void calcFuncMul(uint8_t*);
void calcFuncDiv(uint8_t*);

// global variables
volatile uint8_t blink = (1 << 7);
uint8_t altMode = 0;

// constants
const uint8_t zeroes[8] = { 0 };
uint8_t startup[8] = { SEG_A, 0b01110100, 0b01010000, 0b00110000, SEG_C, SEG_A, 0b00111000, SEG_C };

// the STACK
int64_t stackA = 0;
int64_t stackB = 0;

int main (void) {
   uint8_t buf[8] = { 0 };
   uint32_t keyStates = 0;
   uint32_t prevKeys = 0;
   setup();
   //   intSetup();
   sendBuffer(buf);
   sendBuffer(startup); // comment this out to disable startup message
   _delay_ms(1000);
   while(1) {
      sendByte(0x33); // meaningless data for trigger on logic analyzer
      prevKeys = keyStates; // edge detector or something
      readBytes(&keyStates);
      if((prevKeys != keyStates) && (prevKeys == 0x00)) {
         if(keyStates & NP_NUM) {                  // if numlock is pressed
            int ctr = 0;
            while(keyStates & NP_NUM) {
               ctr++;                              // begin counting held time
               readBytes(&keyStates);              // keep checking if still held
            }
            if (ctr > 300) {                       // if held for a certain amount of time
               altMode |= 0x01;                    // enable altmode
               buf[0] = altMode ? buf[0] | SEGPART_7 : buf[0] & ~SEGPART_7;
               sendBuffer(buf);                    // force refresh display buffer
            } else {
               keyStates |= NP_NUM;                // key not held for long enough, default action
               parseInput(keyStates, buf);         // pass keyStates with button pressed to parseInput
            }
         } else {
            parseInput(keyStates, buf);
         }
      }

      // turn on segments according to content of stack
      buf[0] = stackA ? buf[0] | SEGPART_4 : buf[0] & ~SEGPART_4;
      buf[0] = stackB ? buf[0] | SEGPART_5 : buf[0] & ~SEGPART_5;
      buf[0] = altMode ? buf[0] | SEGPART_7 : buf[0] & ~SEGPART_7;

      sendBuffer(buf);
   }
}

// // ISR with TIMER1_OVF_vect
// // this ISR runs when the timer counter overflows
// // the timer overflows whenever it counts down to INTPER from 2^16
// ISR (TIMER1_OVF_vect) {
//    blink ^= (1 << 7);
//    TCNT1 = INTPER; // 15.8 us for 8MHz clock
// }

// general setup code
void setup() {
   DDRD = SPIALL;
   reset();
   sendByte(0x88);
}

// // interrupt setup code
// void intSetup() {
//    TCNT1 = INTPER;         // counter starts at overflow value
//    TCCR1A = 0x00;          // counter normal mode
//    TCCR1B = (1 << CS11);   // enable pre-scaler (F_CPU/8)
//    TIMSK1 = (1 << TOIE1);  // enable interrupt trigger on overflow
//    sei();                  // global interrupt enable
// }

// put all four bytes of key matrix input into a 32-bit integer
void parseInput(uint32_t keys, uint8_t *b) {
   int nextSeg = 0;
   int isNumber = 0;
   switch (keys) {
      case NP_0:
         isNumber = 1;
         nextSeg = SEG_0;
         break;
      case NP_1:
         // show contents of stackA
         if (altMode) {
            clearBuffer(b);
            putToBuffer(b, stackA);
            altMode = 0;
            break;
         }
         isNumber = 1;
         nextSeg = SEG_1;
         break;
      case NP_2:
         // show contents of stackB
         if (altMode) {
            clearBuffer(b);
            putToBuffer(b, stackB);
            altMode = 0;
            break;
         }
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
         stackInsert(b);
         break;
      case NP_NUM:
         altMode = altMode ? 0 : altMode;
         if (b[7]) {
            clearBuffer(b);
         } else {
            stackA = 0;
            stackB = 0;
            clearBuffer(b);
         }
         break;
      case NP_ADD:
         calcFuncAdd(b);
         break;
      case NP_SUB:
         // code for testing alternate key mode
         if (altMode) {
            clearBuffer(b);
            altMode = 0;
            break;
         }
         calcFuncSub(b);
         break;
      case NP_MUL:
         calcFuncMul(b);
         break;
      case NP_DIV:
         calcFuncDiv(b);
         break;
      case NP_DEC:
         b[7] |= SEGPART_7;
         break;
      default:
         b[7] = b[7];
         break;
   }
   if (isNumber) {
      for (int n = 1; n < 7; n++) {
         b[n] = b[n+1];
      }
      b[7] = nextSeg;
   }
}

void clearBuffer(uint8_t *buf) {
   for (int seg = 0; seg < 8; seg++) {
      buf[seg] = 0;
   }
}

void stackInsert(uint8_t *buf) {
   int64_t currentNumber = parseDisplay(buf);
   if (stackA) {
      stackB = stackA;
      stackA = currentNumber;
   } else {
      stackA = currentNumber;
   }
   clearBuffer(buf);
}

void calcFuncAdd(uint8_t *buf) {
   int64_t res = 0;
   if(!buf[7]) {
      res = stackB + stackA; 
      stackB = 0;
      stackA = res;
   } else {
      res = stackA + parseDisplay(buf);
      stackA = res;
   }
   clearBuffer(buf);
   putToBuffer(buf, res);
}

void calcFuncSub(uint8_t *buf) {
   int64_t res = 0;
   if(!buf[7]) {
      res = stackB - stackA; 
      stackB = 0;
      stackA = res;
   } else {
      res = stackA - parseDisplay(buf);
      stackA = res;
   }
   clearBuffer(buf);
   putToBuffer(buf, res);
}

void calcFuncMul(uint8_t *buf) {
   int64_t res = 0;
   if(!buf[7]) {
      res = stackB * stackA; 
      stackB = 0;
      stackA = res;
   } else {
      res = stackA * parseDisplay(buf);
      stackA = res;
   }
   clearBuffer(buf);
   putToBuffer(buf, res);
}

void calcFuncDiv(uint8_t *buf) {
   int64_t res = 0;
   if(!buf[7]) {
      res = stackB / stackA; 
      stackB = 0;
      stackA = res;
   } else {
      res = stackA / parseDisplay(buf);
      stackA = res;
   }
   clearBuffer(buf);
   putToBuffer(buf, res);
}

// takes a seven segment display character and returns the number it represents
int segToNum(uint8_t seg) {
   switch (seg) {
      case SEG_0:
         return 0;
         break;
      case SEG_1:
         return 1;
         break;
      case SEG_2:
         return 2;
         break;
      case SEG_3:
         return 3;
         break;
      case SEG_4:
         return 4;
         break;
      case SEG_5:
         return 5;
         break;
      case SEG_6:
         return 6;
         break;
      case SEG_7:
         return 7;
         break;
      case SEG_8:
         return 8;
         break;
      case SEG_9:
         return 9;
         break;
      default:
         return 0;
         break;
   }
}

// takes an int and returns its seven segment representation
uint8_t numToSeg(int n) {
   switch (n) {
      case 0:
         return SEG_0;
         break;
      case 1:
         return SEG_1;
         break;
      case 2:
         return SEG_2;
         break;
      case 3:
         return SEG_3;
         break;
      case 4:
         return SEG_4;
         break;
      case 5:
         return SEG_5;
         break;
      case 6:
         return SEG_6;
         break;
      case 7:
         return SEG_7;
         break;
      case 8:
         return SEG_8;
         break;
      case 9:
         return SEG_9;
         break;
      default:
         return 0x00;
         break;
   }
}

// takes the display buffer and converts its contents into a single number
int64_t parseDisplay(uint8_t *buf) {
   int64_t temp = 0;
   for (int seg = 7; seg > 1; seg--) {
      if (buf[seg] != 0x00) {
         temp += segToNum(buf[seg]) * pow(10, 7 - seg);
      }
   }
   return temp;
}

// returns the number at the specified place value
int numberAtPos(int64_t number, int pos) {
   int64_t temp = 0;
   int64_t mulBy = 1;
   for (int ex = 0; ex < pos; ex++) {
      mulBy *= 10;
   }
   temp = number % (mulBy * 10);
   temp /= mulBy;
   return (int) temp;
}

// takes a number i and puts it onto the buffer
void putToBuffer(uint8_t *buf, int64_t i) {
   for (int seg = 7; seg > 0; seg--) {
      buf[seg] = numToSeg(numberAtPos(i,7 - seg));
   }
   for (int seg = 1; seg < 7; seg++) {
      if (buf[seg] == SEG_0) {
         buf[seg] = 0x00;
      } else {
         break;
      }
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
