#include <usr_libc.h>

spin_lock_t uart_put_lock = {0};
spin_lock_t uart_get_lock = {0};
spin_lock_t kb_get_lock = {0};

void usr_uart_put_char(char c)
{
    spin_lock(uart_put_lock);
    if (!PIR1bits.TXIF) {
        INTCONbits.GIE = 0;
        PIE1bits.TXIE = 1;
        tx_wait = get_pid();
        wait_task_info |= (1 << get_pid());
        INTCONbits.GIE = 1;
        set_timer_delay(1);
        while (wait_task_info & (1 << ((run_task_info >> 4) & 0x3)))
            ;
    }
    TXREG = c;
    spin_unlock(uart_put_lock);
}

char usr_uart_get_char()
{
    spin_lock(uart_get_lock);
    char c;
    if (!PIR1bits.RCIF) {
        INTCONbits.GIE = 0;
        PIE1bits.RCIE = 1;
        rc_wait = get_pid();
        wait_task_info |= (1 << get_pid());
        INTCONbits.GIE = 1;
        set_timer_delay(1);
        while (wait_task_info & (1 << ((run_task_info >> 4) & 0x3)))
            ;
    }
    c = RCREG;
    spin_unlock(uart_get_lock);
    return c;
}
