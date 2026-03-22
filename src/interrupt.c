#include <ch375.h>
#include <interrupt.h>
#include <kernel.h>
#include <memory.h>
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
    asm("MOVFF WREG, 0x100\n"
        "MOVFF STATUS, 0x101\n"
        "MOVFF BSR, 0x102\n"
        "MOVFF FSR0L, 0x103\n"
        "MOVFF FSR0H, 0x104\n"
        "MOVFF FSR1L, 0x105\n"
        "MOVFF FSR1H, 0x106\n"
        "MOVFF FSR2L, 0x107\n"
        "MOVFF FSR2H, 0x108\n");

    current->context.wreg = mem._saved_w;
    current->context.status = mem._saved_s;
    current->context.bsr = mem._saved_b;
    current->context.fsr0 = mem._saved_fsr0;
    current->context.fsr1 = mem._saved_fsr1;
    current->context.fsr2 = mem._saved_fsr2;

    current->context.pc.l = TOSL;
    current->context.pc.h = TOSH;
    current->context.pc.u = TOSU;
    asm("POP");

    current->context.rasp = STKPTR;

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
        usb_handler();
        if (kb_info.int_flag) {
            kb_info.int_flag = 0;
            int_wait_queue_pop(0);
        }
        INTCONbits.INT0IF = 0;
    } else if (INTCON3bits.INT1IF) {
        int_wait_queue_pop(1);
        INTCON3bits.INT1IF = 0;
    } else if (INTCON3bits.INT2IF) {
        int_wait_queue_pop(2);
        INTCON3bits.INT2IF = 0;
    } else if (PIR1bits.TXIF && PIE1bits.TXIE) {
        if (tx_wait != 0xFF) {
            wait_task_info ^= 1 << tx_wait;
            tx_wait = 0xFF;
        }
        PIE1bits.TXIE = 0;
    } else if (PIR1bits.RCIF && PIE1bits.RCIE) {
        if (rc_wait != 0xFF) {
            wait_task_info ^= 1 << rc_wait;
            rc_wait = 0xFF;
        }
        PIE1bits.RCIE = 0;
    } else if (INTCONbits.TMR0IF) {
        asm("PICOS_START_SCHEDULE:\n");
        if (run_task_info & RUN_TASK_EXIT) {
            char i = (run_task_info >> 4) & 0x3;
            stack_release(i);
            if (stack_status.wait_pid != 4) {
                run_task[stack_status.wait_pid].stack_info.stack_start =
                    stack_alloc(
                        run_task[stack_status.wait_pid].stack_info.stack_size);

                if (run_task[stack_status.wait_pid].stack_info.stack_start !=
                    4) {
                    stack_status.wait_pid = 4;
                    run_task[stack_status.wait_pid].sp =
                        run_stack[run_task[stack_status.wait_pid]
                                      .stack_info.stack_start];
                }
            }
            if (wait_queue_empty() || stack_status.wait_pid != 4)
                run_task_info ^= (1 << i) | RUN_TASK_EXIT;
            else {
                wait_node_t wait_node;
                wait_queue_out(wait_node);
                run_task[i].context.pc.value = (__uint24) wait_node.func;
                run_task[i].context.rasp = 0;

                run_task[i].stack_info.stack_size = wait_node.stack_size;
                run_task[i].stack_info.stack_start =
                    stack_alloc(wait_node.stack_size);

                if (wait_node.stack_size != 0 &&
                    run_task[i].stack_info.stack_start == 4)
                    stack_status.wait_pid = i;
                else
                    run_task[i].sp =
                        run_stack[run_task[i].stack_info.stack_start];
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

    PROD = current->context.prod;

    TABLAT = current->context.tablat;

    TBLPTRL = current->context.tblptrl;
    TBLPTRH = current->context.tblptrh;
    TBLPTRU = current->context.tblptru;

    EECON1 = current->context.eecon1;

    mem._saved_w = current->context.wreg;
    mem._saved_s = current->context.status;
    mem._saved_b = current->context.bsr;
    mem._saved_fsr0 = current->context.fsr0;
    mem._saved_fsr1 = current->context.fsr1;
    mem._saved_fsr2 = current->context.fsr2;

    asm("MOVFF 0x107, FSR2L\n"
        "MOVFF 0x108, FSR2H\n"
        "MOVFF 0x105, FSR1L\n"
        "MOVFF 0x106, FSR1H\n"
        "MOVFF 0x103, FSR0L\n"
        "MOVFF 0x104, FSR0H\n"
        "MOVFF 0x102, BSR\n"
        "MOVFF 0x101, STATUS\n"
        "MOVFF 0x100, WREG\n"
        "RETFIE\n");
}
