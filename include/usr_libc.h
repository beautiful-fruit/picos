#include <interrupt.h>
#include <kernel.h>
#include <libc.h>
#include <lock.h>
#include <schedule.h>

extern spin_lock_t uart_put_lock;

#define usr_uart_put_char(c)                                             \
    do {                                                                 \
        spin_lock(uart_put_lock);                                        \
        if (!PIR1bits.TXIF) {                                            \
            INTCONbits.GIE = 0;                                          \
            PIE1bits.TXIE = 1;                                           \
            tx_wait = get_pid();                                         \
            wait_task_info |= (1 << get_pid());                          \
            INTCONbits.GIE = 1;                                          \
            set_timer_delay(1);                                          \
            while (wait_task_info & (1 << ((run_task_info >> 4) & 0x3))) \
                ;                                                        \
        }                                                                \
        TXREG = c;                                                       \
        spin_unlock(uart_put_lock);                                      \
    } while (0)
