#include <xc.h>

#pragma config OSC = HS
#pragma config WDT = OFF
#pragma config LVP = OFF
#pragma config PBADEN = OFF

#define _XTAL_FREQ 40000000ULL

void uart_init(void);

inline void uart_putchar(char c);

inline char uart_getchar(void);
