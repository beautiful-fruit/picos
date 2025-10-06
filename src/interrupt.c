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

    context.pcl = TOSL;
    context.pch = TOSH;
    context.pcu = TOSU;
    asm("POP");

    context.rasp = STKPTR;

    if (STKPTR & 0x1F) {
        context.ral = TOSL;
        context.rah = TOSH;
        context.rau = TOSU;
        asm("POP");
    }

    if (INTCONbits.TMR0IF) {
        INTCONbits.TMR0IF = 0;
        printf("timer interrupt\n");
        set_timer_delay(ONE_SEC);
    }

    if (context.rasp & 0x1F) {
        asm("PUSH");
        TOSL = context.ral;
        TOSH = context.rah;
        TOSU = context.rau;
    }
    asm("PUSH");
    TOSL = context.pcl;
    TOSH = context.pch;
    TOSU = context.pcu;

    STATUS = context.status;
    WREG = context.wreg;
    BSR = context.bsr;

    asm("RETFIE");
}