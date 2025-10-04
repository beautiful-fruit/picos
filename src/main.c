#include <interrupt.h>
#include <libc.h>


void main(void)
{
    uart_init();
    INTCONbits.GIE = 1;
    timer0_init();
    set_timer_delay(ONE_SEC);
    while (1)
        ;
    PANIC("hello\n");
}
