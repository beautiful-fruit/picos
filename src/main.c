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
    lock();
    uart_putchar('?');
    uart_putchar('1');
    uart_putchar('\r');
    uart_putchar('\n');
    unlock();
    exit();
}

void __attribute__((naked)) task2(void)
{
    lock();
    uart_putchar(':');
    uart_putchar('2');
    uart_putchar('\r');
    uart_putchar('\n');
    unlock();
    current->sp += 2;
#define result (*(current->sp - 2))
#define i (*(current->sp - 1))

    for (i = 0; i < 255; i++) {
        while (1) {
            lock();
            result = create_process(&task1);
            unlock();
            if (result)
                break;
        }
    }
    lock();
    uart_putchar('>');
    uart_putchar('\r');
    uart_putchar('\n');
    unlock();
#undef result
#undef i
    exit();
}

void __attribute__((naked)) task3(void)
{
    while (1) {
        int_wait_queue_push(0);
        lock();
        uart_putchar('0');
        unlock();
    }
    exit();
}

void main(void)
{
    // Make the return address stack empty
    STKPTR &= 0xE0;
    uart_init();
    extern_memory_init();
    INTCONbits.GIE = 1;

    ADCON1 = 0xF;
    PORTB = 0;
    TRISB = 1;
    INTCONbits.INT0IF = 0;
    INTCONbits.INT0IE = 1;

    timer0_init();
    create_process(&task3);
    create_process(&task2);
    start_schedule();
    PANIC("hello\n");
}
