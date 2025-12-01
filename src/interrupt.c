#include <interrupt.h>
#include <kernel.h>
#include <schedule.h>
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

void __attribute__((naked)) isr(void)
{
    current->context.status = STATUS;
    current->context.wreg = WREG;
    current->context.bsr = BSR;

    current->context.pc.l = TOSL;
    current->context.pc.h = TOSH;
    current->context.pc.u = TOSU;
    asm("POP");

    current->context.rasp = STKPTR;

    current->context.fsr0 = FSR0;
    current->context.fsr1 = FSR1;
    current->context.fsr2 = FSR2;

    current->context.prod = PROD;

    current->context.tablat = TABLAT;

    current->context.tblptrl = TBLPTRL;
    current->context.tblptrh = TBLPTRH;
    current->context.tblptru = TBLPTRU;

    current->context.eecon1 = EECON1;

    if (STKPTR & 0x1F) {
        *(current->sp - 3) = TOSL;
        *(current->sp - 2) = TOSH;
        *(current->sp - 1) = TOSU;
        asm("POP");
    }

    /* trap handler start */
    if (INTCONbits.INT0IF) {
        int_wait_queue_pop(0);
        INTCONbits.INT0IF = 0;
    } else if (INTCON3bits.INT1IF) {
        int_wait_queue_pop(1);
        INTCON3bits.INT1IF = 0;
    } else if (INTCON3bits.INT2IF) {
        int_wait_queue_pop(2);
        INTCON3bits.INT2IF = 0;
    } else if (INTCONbits.TMR0IF) {
        asm("PICOS_START_SCHEDULE:\n");
        if (run_task_info & RUN_TASK_EXIT) {
            char i = (run_task_info >> 4) & 0x3;
            if (wait_queue_empty())
                run_task_info ^= (1 << i) | RUN_TASK_EXIT;
            else {
                func_t func;
                wait_queue_out(func);
                run_task[i].sp = run_stack[i];
                run_task[i].context.pc.value = (__uint24) func;
                run_task[i].context.rasp = 0;
            }
        }
        INTCONbits.TMR0IF = 0;
        current = schedule();
        set_timer_delay(ONE_SEC / 100);
    }
    /* trap handler end */

    if (current->context.rasp & 0x1F) {
        asm("PUSH");
        TOSL = *(current->sp - 3);
        TOSH = *(current->sp - 2);
        TOSU = *(current->sp - 1);
    }
    asm("PUSH");
    TOSL = current->context.pc.l;
    TOSH = current->context.pc.h;
    TOSU = current->context.pc.u;

    FSR0 = current->context.fsr0;
    FSR1 = current->context.fsr1;
    FSR2 = current->context.fsr2;

    PROD = current->context.prod;

    TABLAT = current->context.tablat;

    TBLPTRL = current->context.tblptrl;
    TBLPTRH = current->context.tblptrh;
    TBLPTRU = current->context.tblptru;

    EECON1 = current->context.eecon1;

    STATUS = current->context.status;
    WREG = current->context.wreg;
    BSR = current->context.bsr;

    asm("RETFIE");
}