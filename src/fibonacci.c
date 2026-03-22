#include <fibonacci.h>
#include <interrupt.h>
#include <lock.h>
#include <schedule.h>
#include <usr_libc.h>

#define MOD 1000000007UL

spin_lock_t fp_lock = {0};

uint32_t fsstart_ans = 0;
uint32_t fsstart_param = 0;

void __attribute__((naked)) fpstart(void)
{
    spin_lock(fp_lock);
    usr_uart_put_char('f');
    usr_uart_put_char('\n');
    uint32_t n = fsstart_param;
    if (n == 0)
        fsstart_ans = 0;
    else if (n == 1)
        fsstart_ans = 1;
    else {
        uint32_t a = 0;
        uint32_t b = 1;
        uint32_t c = 0;
        for (uint32_t i = 2; i <= n; i++) {
            uint32_t block_end = (i + 49 < n) ? i + 49 : n;
            for (uint32_t j = i; j <= block_end; j++) {
                lock();
                c = (a + b) % MOD;
                unlock();
                a = b;
                b = c;
            }
            i = block_end;
        }
        fsstart_ans = c;
    }
    spin_unlock(fp_lock);
    exit();
}

void __attribute__((naked)) fpend(void)
{
    uint8_t result;
    uint8_t i;

    try_spin_lock(fp_lock, result);
    if (result) {
        /* get lock */

        usr_uart_put_char('r');
        usr_uart_put_char(':');

        static char num_str[16];

        if (fsstart_ans == 0)
            usr_uart_put_char('0');
        else {
            i = 0;
            while (fsstart_ans > 0) {
                lock();
                num_str[i++] = (fsstart_ans % 10) + '0';
                fsstart_ans /= 10;
                unlock();
            }
            for (; i > 0; i--)
                usr_uart_put_char(num_str[i - 1]);
        }
        usr_uart_put_char('\r');
        usr_uart_put_char('\n');
        usr_uart_put_char('$');
        usr_uart_put_char(' ');

        fsstart_ans = 0;
        spin_unlock(fp_lock);
    } else {
        char *st = "error: calculation not finished\r\n$ ";
        for (i = 0; i < sizeof("error: calculation not finished\r\n$ "); i++)
            usr_uart_put_char(st[i]);
    }

    exit();
}
