#include <xc.h>

#pragma config OSC = HS
#pragma config WDT = OFF
#pragma config LVP = OFF
#pragma config PBADEN = OFF

#ifdef EXTERNAL_CLOCK
#define _XTAL_FREQ 40000000ULL
#define ONE_SEC 39063
#else
#define _XTAL_FREQ 4000000ULL
#define ONE_SEC 62500
#endif

void uart_init(void);

inline void uart_putchar(char c);

inline char uart_getchar(void);
