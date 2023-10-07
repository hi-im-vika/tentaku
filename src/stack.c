#include "stack.h"

void stackInsert(uint8_t *buf, int64_t *stackA, int64_t *stackB) {
   int64_t currentNumber = parseDisplay(buf);
   if (stackA) {
      *stackB = *stackA;
      *stackA = currentNumber;
   } else {
      *stackA = currentNumber;
   }
   clearBuffer(buf);
}
