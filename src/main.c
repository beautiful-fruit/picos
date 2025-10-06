#include <interrupt.h>
#include <kernel.h>
#include <libc.h>

char ustack0[256] __at(0x256);
char *sp = NULL;

#define test_add(arg1, arg2, output) \
    do {                             \
        sp += 6;                     \
        *(sp - (6 - 3)) = arg1;      \
        *(sp - (6 - 4)) = arg2;      \
        asm("CALL _test_add_impl");  \
        output = *(sp - (6 - 5));    \
        sp -= 6;                     \
    } while (0)

void __attribute__((naked)) test_add_impl(void)
{
#define ret (*(sp - (6 - 5)))
#define arg1 (*(sp - (6 - 3)))
#define arg2 (*(sp - (6 - 4)))
    enter_user_func(6);
    ret = arg1 + arg2;
#undef ret
#undef arg1
#undef arg2
    return_user_func(6);
}

void main(void)
{
    sp = ustack0;
    uart_init();
    INTCONbits.GIE = 1;
    timer0_init();
    set_timer_delay(ONE_SEC);
    while (1) {
        char ans;
        test_add(1, 1, ans);
        timer_disable();
        printf("meow: %d\n", ans);
        __delay_ms(1000);
        timer_enable();
    }
    PANIC("hello\n");
}
