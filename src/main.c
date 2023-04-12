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
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include "main.h"

#define STB PORTD7
#define CLK PORTD6
#define DIO PORTD5
#define DTIME 5

// #define INTPER 65523 // through trial and error, this one is about 32 kHz
                     // about 15 us of thinking time !
#define INTPER 1

void reset();
void intsetup();

unsigned char tog = (1 << 7);
unsigned char bytebuf = 0;

int main (void) {
//   unsigned char buf[8] = { chars[1], chars[2], chars[3], chars[4], chars[10], chars[11], chars[12], chars[13], };
   intsetup();
   while(1) {
      DDRD = 0b11100000;
      reset();
   }
}
// ISR with TIMER1_OVF_vect
// this ISR runs when the timer counter overflows
// the timer overflows whenever it counts down to INTPER from 2^16
ISR (TIMER1_OVF_vect) {
   PORTB ^= (1 << PORTB3); // Toggle the 5th data register of PORTB
   tog ^= (1 << 7);
   TCNT1 = INTPER; // 15.8 us for 8MHz clock
}

// interrupt setup code
void intsetup() {
   DDRB = (1 << DDB3); // Set 5th data direction register of PORTB. A set value means output
   TCNT1 = INTPER; // 15.8 us for 8MHz clock
   TCCR1A = 0x00; // Set normal counter mode
   TCCR1B = (1<<CS11); // Set 8 pre-scaler
   TIMSK1 = (1 << TOIE1); // Set overflow interrupt enable bit
   sei(); // Enable interrupts globally
}

void reset() {
   PORTD |= ((1<<CLK) | (1<<STB)); // bring CLK and STB high
}
