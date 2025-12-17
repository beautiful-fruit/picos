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

void usr_uart_put_char(char c);

char usr_uart_get_char();

#define usr_kb_get_char(c)          \
    do {                            \
        spin_lock(kb_get_lock);     \
        if (kb_queue_empty())       \
            int_wait_queue_push(0); \
        kb_queue_out(c);            \
        spin_unlock(kb_get_lock);   \
    } while (0)
