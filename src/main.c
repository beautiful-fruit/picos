#include <interrupt.h>
#include <libc.h>


void main(void)
{
    uart_init();
    INTCONbits.GIE = 1;
    timer0_init();
    set_timer_delay(ONE_SEC);
    while (1) {
        timer_disable();
        printf("meow\n");
        __delay_ms(1000);
        timer_enable();
    }
    PANIC("hello\n");
}
