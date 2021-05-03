#ifndef __SIMPLE_RV32I_UTILS_H__
#define __SIMPLE_RV32I_UTILS_H__
#include <cstdarg>
#include <cstdlib>
#include <stdint.h>
#include <stdio.h>

#define DEBUG_NONE 0
#define DEBUG_LOW 1
#define DEBUG_MEDIUM 2
#define DEBUG_HIGH 3

void debug_printf(uint8_t debugLevel, const char * format, ... );

#endif
