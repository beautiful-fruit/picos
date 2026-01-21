#pragma once
#include <ch375.h>
#include <interrupt.h>
#include <kernel.h>
#include <libc.h>
#include <lock.h>
#include <schedule.h>

extern spin_lock_t uart_put_lock;
extern spin_lock_t uart_get_lock;
extern spin_lock_t kb_get_lock;

// void usr_uart_put_char(char c);

// char usr_uart_get_char();

#define usr_kb_get_char(c)          \
    do {                            \
        spin_lock(kb_get_lock);     \
        if (kb_queue_empty())       \
            int_wait_queue_push(0); \
        kb_queue_out(c);            \
        spin_unlock(kb_get_lock);   \
    } while (0)

#define usr_uart_put_char(c)                                             \
    do {                                                                 \
        GIE = 0;\
        putchar(c);\
        GIE = 1;\
    } while (0)

#define usr_uart_get_char(c)                                             \
    do {                                                                 \
        spin_lock(uart_get_lock);                                        \
        INTCONbits.GIE = 0;                                          \
        if (!PIR1bits.RCIF) {                                            \
            PIE1bits.RCIE = 1;                                           \
            rc_wait = get_pid();                                         \
            wait_task_info |= (1 << get_pid());                          \
            set_timer_delay(1);                                          \
            INTCONbits.GIE = 1;                                          \
            while (wait_task_info & (1 << ((run_task_info >> 4) & 0x3))) \
                ;                                                        \
        }                                                                \
        set_timer_delay(1);                                          \
        c = RCREG;                                                       \
        spin_unlock(uart_get_lock);                                      \
    } while (0)
