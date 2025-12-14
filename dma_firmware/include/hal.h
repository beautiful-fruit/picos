#pragma once
#ifdef __XC8
#include <xc.h>
#else
#include <stdint.h>
#include "dfp/xc8/pic/include/proc/pic18f4520.h"
#define bit unsigned char
#define bool unsigned char
#define NULL 0
#endif

#pragma config OSC = HS
#pragma config WDT = OFF
#pragma config LVP = OFF
#pragma config PBADEN = OFF

#define _XTAL_FREQ 40000000ULL

void uart_init(void);

inline void uart_putchar(char c);

inline char uart_getchar(void);

void extern_memory_init(void);


/**
 * NOTICE: extern memory read/write have to be called with interrupt turned
 * off.
 */
void extern_memory_write(uint32_t addr, uint8_t c);

uint8_t extern_memory_read(uint32_t addr);
