#include <interrupt.h>
#include <kernel.h>
#include <libc.h>
#include <schedule.h>

#define test_add(arg1, arg2, output) \
    do {                             \
        current->sp += 6;            \
        *(current->sp - 6) = arg1;   \
        *(current->sp - 5) = arg2;   \
        asm("CALL _test_add_impl");  \
        output = *(current->sp - 4); \
        current->sp -= 6;            \
    } while (0)

void __attribute__((naked)) test_add_impl(void)
{
#define ret (*(current->sp - 4))
#define arg1 (*(current->sp - 6))
#define arg2 (*(current->sp - 5))
    enter_user_func();
    ret = arg1 + arg2;
#undef ret
#undef arg1
#undef arg2
    return_user_func();
}

void __attribute__((naked)) task1(void)
{
    while (1) {
        timer_disable();
        uart_putchar('1');
        uart_putchar('\r');
        uart_putchar('\n');
        timer_enable();
    }
}

void __attribute__((naked)) task2(void)
{
    while (1) {
        timer_disable();
        uart_putchar('2');
        uart_putchar('\r');
        uart_putchar('\n');
        timer_enable();
    }
}

void main(void)
{
    // Make the return address stack empty
    STKPTR &= 0xE0;
    uart_init();
    INTCONbits.GIE = 1;
    timer0_init();

    create_process(&task1);
    create_process(&task2);
    create_process(&task1);
    create_process(&task2);

    start_schedule();
    PANIC("hello\n");
}
