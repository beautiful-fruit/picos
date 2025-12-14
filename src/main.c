#include <ch375.h>
#include <dma.h>
#include <interrupt.h>
#include <kernel.h>
#include <libc.h>
#include <schedule.h>
#include <tests.h>

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
    uart_putchar('0' + ((run_task_info >> 4) & 0x3));
    uart_putchar(':');
    unlock();
    lock();
    uart_putchar('x');
    uart_putchar('\r');
    uart_putchar('\n');
    unlock();
    exit();
}

void __attribute__((naked)) task2(void)
{
    lock();
    uart_putchar('0' + ((run_task_info >> 4) & 0x3));
    uart_putchar(':');
    unlock();
    lock();
    uart_putchar('2');
    uart_putchar('\r');
    uart_putchar('\n');
    unlock();
    exit();
}

void __attribute__((naked)) task3(void)
{
    while (1) {
        int_wait_queue_push(0);
        lock();
        uart_putchar('3');
        unlock();
    }
    exit();
}

void __attribute__((naked)) task4(void)
{
    while (1) {
        int_wait_queue_push(0);
        lock();
        uart_putchar('4');
        unlock();
    }
    exit();
}

void main(void)
{
    // Make the return address stack empty
    GIE = 0;
    STKPTR &= 0xE0;
    uart_init();
    dma_init();
    extern_memory_init();
    ch375_init();
    __delay_ms(3000);


    ADCON1 = 0xF;
    timer0_init();

    init_scheduler();

    create_process(&task3, 10);  // this will fail to create process
    create_process(&task1, 2);
    create_process(&task2, 2);
    create_process(&task1, 1);
    create_process(&task1, 2);
    create_process(&task2, 0);
    create_process(&task2, 0);

    create_process(&task3, 0);
    create_process(&task4, 0);

    start_schedule();
    PANIC("hello\n");
}
