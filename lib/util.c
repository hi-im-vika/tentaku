#include "util.h"

// put all four bytes of key matrix input into a 32-bit integer
// uint32_t    keys        integer with keys pressed
// uint8_t     *b          pointer to display buffer
// uint8_t     *altMode    pointer to altMode indicator
// int64_t     *stackA     signed integer for stack A contents
// int64_t     *stackB     signed integer for stack B contents
void parseInput(uint32_t keys, uint8_t *b, uint8_t *altMode, int64_t *stackA, int64_t *stackB) {
   int nextSeg = 0;
   int isNumber = 0;
   switch (keys) {
      case NP_0:
         isNumber = 1;
         nextSeg = SEG_0;
         break;
      case NP_1:
         // show contents of stackA
         if (*altMode) {
            clearBuffer(b);
            putToBuffer(b, *stackA);
            *altMode = 0;
            break;
         }
         isNumber = 1;
         nextSeg = SEG_1;
         break;
      case NP_2:
         // show contents of stackB
         if (*altMode) {
            clearBuffer(b);
            putToBuffer(b, *stackB);
            *altMode = 0;
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
         stackInsert(b, stackA, stackB);
         break;
      case NP_NUM:
         altMode = altMode ? 0 : altMode;
         if (b[7]) {
            clearBuffer(b);
         } else {
            *stackA = 0;
            *stackB = 0;
            clearBuffer(b);
         }
         break;
      case NP_ADD:
         calcFuncAdd(b, stackA, stackB);
         break;
      case NP_SUB:
         // code for testing alternate key mode
         if (altMode) {
            clearBuffer(b);
            altMode = 0;
            break;
         }
         calcFuncSub(b, stackA, stackB);
         break;
      case NP_MUL:
         calcFuncMul(b, stackA, stackB);
         break;
      case NP_DIV:
         calcFuncDiv(b, stackA, stackB);
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
