#include <ch375.h>
#include <interrupt.h>
#include <kernel.h>
#include <libc.h>
#include <lock.h>
#include <schedule.h>

extern spin_lock_t uart_put_lock;
extern spin_lock_t uart_get_lock;
extern spin_lock_t kb_get_lock;

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

#define usr_uart_get_char(c)                                             \
    do {                                                                 \
        spin_lock(uart_get_lock);                                        \
        if (!PIR1bits.RCIF) {                                            \
            INTCONbits.GIE = 0;                                          \
            PIE1bits.RCIE = 1;                                           \
            rc_wait = get_pid();                                         \
            wait_task_info |= (1 << get_pid());                          \
            INTCONbits.GIE = 1;                                          \
            set_timer_delay(1);                                          \
            while (wait_task_info & (1 << ((run_task_info >> 4) & 0x3))) \
                ;                                                        \
        }                                                                \
        c = RCREG;                                                       \
        spin_unlock(uart_get_lock);                                      \
    } while (0)

#define usr_kb_get_char(c)          \
    do {                            \
        spin_lock(kb_get_lock);     \
        if (kb_queue_empty())       \
            int_wait_queue_push(0); \
        kb_queue_out(c);            \
        spin_unlock(kb_get_lock);   \
    } while (0)
