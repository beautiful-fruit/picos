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

#pragma interrupt_level 1
#pragma interrupt_level 2
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

void extern_memory_init(void)
{
    /**
     * RA2 and RA3 are the inputs of the decoder
     * The gray code is used here to ensure two ajacent states are different
     * only by one bit.
     *
     * The order of states are defined as follows:
     * -------------------------
     * | RA3 | RA2 |  meaning  |
     * -------------------------
     * |  0  |  0  |  latch 2  |
     * |  0  |  1  |  latch 1  |
     * |  1  |  1  |  latch 0  |
     * |  1  |  0  |     x     |
     * -------------------------
     *
     * 00 is the initial state
     */
    asm(
        "CLRF TRISD\n"
        "BCF TRISA, 0\n"
        "BCF TRISA, 1\n"
        "BCF TRISA, 2\n"
        "BCF TRISA, 3\n"
        "BSF LATA, 0\n"
        "BSF LATA, 1\n"
        "BCF LATA, 2\n"
        "BCF LATA, 3\n");
}

void extern_memory_write(uint32_t addr, uint8_t c)
{
    asm("CLRF TRISD\n");
    LATD = (addr >> 16) & 0xFF;
    asm(
        "NOP\n"
        "BSF LATA, 2\n"
        "NOP\n");
    LATD = (addr >> 8) & 0xFF;
    asm(
        "NOP\n"
        "BSF LATA, 3\n"
        "NOP\n");
    LATD = addr & 0xFF;
    asm(
        "NOP\n"
        "BCF LATA, 2\n"
        "NOP\n");
    LATD = c;
    asm(
        "NOP\n"
        "BCF LATA, 0\n"
        "BSF LATA, 0\n"
        "BCF LATA, 3\n"
    );
}

uint8_t extern_memory_read(uint32_t addr)
{
    asm("CLRF TRISD\n");
    LATD = (addr >> 16) & 0xFF;
    asm(
        "NOP\n"
        "BSF LATA, 2\n"
        "NOP\n");
    LATD = (addr >> 8) & 0xFF;
    asm(
        "NOP\n"
        "BSF LATA, 3\n"
        "NOP\n");
    LATD = addr & 0xFF;
    asm(
        "NOP\n"
        "BCF LATA, 2\n"
        "NOP\n"
        "SETF TRISD\n"
        "BCF LATA, 1\n"
        "NOP\n"
    );
    uint8_t x = PORTD;
    asm(
        "BSF LATA, 1\n"
        "BCF LATA, 3\n"
        "CLRF TRISD\n"
    );
}
