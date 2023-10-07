#include "calcfunc.h"

void calcFuncAdd(uint8_t *buf, int64_t *stackA, int64_t *stackB) {
   int64_t res = 0;
   if(!buf[7]) {
      res = *stackB + *stackA; 
      *stackB = 0;
      *stackA = res;
   } else {
      res = *stackA + parseDisplay(buf);
      *stackA = res;
   }
   clearBuffer(buf);
   putToBuffer(buf, res);
}

void calcFuncSub(uint8_t *buf, int64_t *stackA, int64_t *stackB) {
   int64_t res = 0;
   if(!buf[7]) {
      res = *stackB - *stackA; 
      *stackB = 0;
      *stackA = res;
   } else {
      res = *stackA - parseDisplay(buf);
      *stackA = res;
   }
   clearBuffer(buf);
   putToBuffer(buf, res);
}

void calcFuncMul(uint8_t *buf, int64_t *stackA, int64_t *stackB) {
   int64_t res = 0;
   if(!buf[7]) {
      res = *stackB * *stackA; 
      *stackB = 0;
      *stackA = res;
   } else {
      res = *stackA * parseDisplay(buf);
      *stackA = res;
   }
   clearBuffer(buf);
   putToBuffer(buf, res);
}

void calcFuncDiv(uint8_t *buf, int64_t *stackA, int64_t *stackB) {
   int64_t res = 0;
   if(!buf[7]) {
      res = *stackB / *stackA; 
      *stackB = 0;
      *stackA = res;
   } else {
      res = *stackA / parseDisplay(buf);
      *stackA = res;
   }
   clearBuffer(buf);
   putToBuffer(buf, res);
}
