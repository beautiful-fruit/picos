#include <libc.h>
#include <stdarg.h>

#define set_timer_delay(t)             \
    T0CONbits.TMR0ON = 0;              \
    TMR0H = ((65536 - t) >> 8) & 0xFF; \
    TMR0L = (65536 - t) & 0xFF;        \
    T0CONbits.TMR0ON = 1;

inline void timer0_Init(void)
{
#ifdef EXTERNAL_CLOCK  // 40M
    T0CONbits.T0CS = 0;
    T0CONbits.T0PS = 0b111;  // 256
#else                        // 4M
    T0CONbits.T0CS = 0;
    T0CONbits.T0PS = 0b011;  // 16
#endif

    T0CONbits.T08BIT = 0;  // 16 bits timer
    T0CONbits.PSA = 0;
    INTCONbits.TMR0IF = 0;
    INTCONbits.TMR0IE = 1;
}

void __attribute__((naked)) isr(void)
{
    if (INTCONbits.TMR0IF) {
        INTCONbits.TMR0IF = 0;
        printf("timer interrupt\n");
        set_timer_delay(ONE_SEC);
    }
    asm("RETFIE");
}

void main(void)
{
    uart_init();
    TRISBbits.TRISB0 = 0;
    LATBbits.LATB0 = 0;
    INTCONbits.GIE = 1;
    timer0_Init();
    set_timer_delay(ONE_SEC);
    while (1)
        ;
    PANIC("hello\n");
}
