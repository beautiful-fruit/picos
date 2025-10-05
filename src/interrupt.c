#include <interrupt.h>
#include <type.h>

inline void timer0_init(void)
{
#ifdef EXTERNAL_CLOCK        // 40M
    T0CONbits.T0PS = 0b111;  // 256
#else                        // 4M
    T0CONbits.T0PS = 0b011;  // 16
#endif
    T0CONbits.T0CS = 0;
    T0CONbits.T08BIT = 0;  // 16 bits timer
    T0CONbits.PSA = 0;
    INTCONbits.TMR0IF = 0;
    INTCONbits.TMR0IE = 1;
}

static Context context __at(0x0);

void __attribute__((naked)) isr(void)
{
    context.status = STATUS;
    context.wreg = WREG;
    context.bsr = BSR;
    if (INTCONbits.TMR0IF) {
        INTCONbits.TMR0IF = 0;

        printf("status: %x, wreg: %x, bsr: %x\n", context.status, context.wreg,
               context.bsr);
        printf("stack top: 0x%x%x%x\n", TOSU, TOSH, TOSL);

        printf("timer interrupt\n");
        set_timer_delay(ONE_SEC);
        STATUS = context.status;
        WREG = context.wreg;
        BSR = context.bsr;
    }
    asm("RETFIE");
}