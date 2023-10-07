#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include <util/delay.h>
#include <avr/io.h>
#include "consts.h"

#ifdef SPI_IMPORT
   #define EXTERN
#else
   #define EXTERN extern
#endif

EXTERN void sendByte(uint8_t);
EXTERN void readBytes(uint32_t*);
EXTERN void shiftOut(int, uint8_t);
EXTERN void reset(void);

#undef SPI_IMPORT
#undef EXTERN
#endif
