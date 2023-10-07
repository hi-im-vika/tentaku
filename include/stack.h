#ifndef STACK_H
#define STACK_H

#include <stdint.h>
#include "util.h"
#include "buffer.h"

#ifdef STACK_IMPORT
   #define EXTERN
#else
   #define EXTERN extern
#endif

EXTERN void stackInsert(uint8_t*, int64_t*, int64_t*);

#undef STACK_IMPORT
#undef EXTERN
#endif
