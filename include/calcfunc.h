
#ifndef CALCFUNC_H
#define CALCFUNC_H

#include <stdint.h>
#include "buffer.h"
#include "util.h"

#ifdef CALCFUNC_IMPORT
   #define EXTERN
#else
   #define EXTERN extern
#endif

EXTERN void calcFuncAdd(uint8_t*, int64_t*, int64_t*);
EXTERN void calcFuncSub(uint8_t*, int64_t*, int64_t*);
EXTERN void calcFuncMul(uint8_t*, int64_t*, int64_t*);
EXTERN void calcFuncDiv(uint8_t*, int64_t*, int64_t*);

#undef CALCFUNC_IMPORT
#undef EXTERN
#endif
