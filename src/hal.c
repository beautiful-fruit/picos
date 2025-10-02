#include <hal.h>

void uart_init(void)
{
    unsigned long baud = 9600;
    unsigned long spbrg;

    TXSTAbits.SYNC = 0;
    TXSTAbits.BRGH = 1;
    BAUDCONbits.BRG16 = 1;

    spbrg = (_XTAL_FREQ / (4UL * baud)) - 1;

    SPBRG = spbrg & 0xFF;
    SPBRGH = (spbrg >> 8) & 0xFF;

    RCSTAbits.SPEN = 1;
    TXSTAbits.TXEN = 1;
    RCSTAbits.CREN = 1;
}

inline void uart_putchar(char c)
{
    while (!TXSTAbits.TRMT)
        ;
    TXREG = c;
}

inline char uart_getchar(void)
{
    while (!PIR1bits.RCIF)
        ;
    return RCREG;
}