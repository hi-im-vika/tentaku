// main.c
// tentaku_mod
// gotta use bitwise ops to do bit manupulation
// who woulda guessed
// PORTD5: DIO (VIO2)
// PORTD6: CLK (GRN2)
// PORTD7: STB (PCH2)
// SPI: PCH GRN VIO BRN BLK RED
// vika

#include "main.h"

// function prototypes
void setup();
void intSetup();
void reset();

// global variables
// volatile uint8_t blink = (1 << 7);

// constants
const uint8_t zeroes[8] = { 0 };
// uint8_t startup[8] = { SEG_A, 0b01110100, 0b01010000, 0b00110000, SEG_C, SEG_A, 0b00111000, SEG_C };
uint8_t startup[8] = { 0b01010000, 0b01110011, 0b01010100, SEG_C, SEG_A, 0b00111000, SEG_C, 0 };   // rPnCALC


int main (void) {
   
   // altmode indicator
   uint8_t altMode = 0;

   // the STACK
   int64_t stackA = 0;
   int64_t stackB = 0;

   uint8_t buf[8] = { 0 };
   uint32_t keyStates = 0;    // 32-bit integer to store current keys pressed
   uint32_t prevKeys = 0;     // 32-bit integer to store last keys pressed
   setup();
   //   intSetup();
   sendBuffer(buf);
   sendBuffer(startup); // comment this out to disable startup message
   _delay_ms(1000);
   while(1) {
      sendByte(0x33); // meaningless data for trigger on logic analyzer
      prevKeys = keyStates; // edge detector or something
      readBytes(&keyStates);
      if((prevKeys != keyStates) && (prevKeys == 0x00)) {   // if current keys pressed not equal to last keys pressed
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
               parseInput(keyStates, buf, &altMode, &stackA, &stackB);         // pass keyStates with button pressed to parseInput
            }
         } else {
            parseInput(keyStates, buf, &altMode, &stackA, &stackB);
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
