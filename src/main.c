#include <interrupt.h>
#include <kernel.h>
#include <libc.h>
#include <schedule.h>

#define test_add(arg1, arg2, output) \
    do {                             \
        *(current->sp) = arg1;       \
        *(current->sp + 1) = arg2;   \
        current->sp += 6;            \
        asm("CALL _test_add_impl");  \
        current->sp -= 6;            \
        output = *(current->sp + 2); \
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
    current->sp += 2;
#define result (*(current->sp - 2))
#define i (*(current->sp - 1))

    for (; i < 5; i++) {
        test_add(i, i, result);
        timer_disable();
        uart_putchar(':');
        uart_putchar(result + '0');
        uart_putchar('\r');
        uart_putchar('\n');
        timer_enable();
    }

#undef result
    exit();
}

void __attribute__((naked)) task2(void)
{
    timer_disable();
    uart_putchar('2');
    uart_putchar('\r');
    uart_putchar('\n');
    timer_enable();

    exit();
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
