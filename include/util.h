#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include "consts.h"
#include "calcfunc.h"
#include "stack.h"
#include "buffer.h"

#ifdef UTIL_IMPORT
   #define EXTERN
#else
   #define EXTERN extern
#endif

EXTERN void parseInput(uint32_t, uint8_t*, uint8_t*, int64_t*, int64_t*);
EXTERN int64_t parseDisplay(uint8_t*);
EXTERN int segToNum(uint8_t);
EXTERN uint8_t numToSeg(int);
EXTERN int numberAtPos(int64_t, int);

#undef UTIL_IMPORT
#undef EXTERN
#endif
