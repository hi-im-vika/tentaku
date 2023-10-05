#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>
#include <util/delay.h>
#include <avr/io.h>
#include "util.h"
#include "spi.h"
#include "consts.h"

#ifdef BUFFER_IMPORT
   #define EXTERN
#else
   #define EXTERN extern
#endif

EXTERN void sendBuffer(uint8_t*);
EXTERN void putToBuffer(uint8_t*, int64_t);
EXTERN void clearBuffer(uint8_t*);

#undef BUFFER_IMPORT
#undef EXTERN
#endif
