#pragma once
#include <interrupt.h>
#include <kernel.h>

/**
 * create_process - create a new process
 * @func: The function that the process will execute
 * @stack_size: indicates that the stack size is 128 multiplied by stack_size
 *
 * Return 1 if success, return 0 if fail.
 */
char create_process(void (*func)(void), uint8_t stack_size);

uint8_t stack_alloc(uint8_t stack_size);

void stack_release(uint8_t pid);

Task *schedule();

void init_scheduler();

#define start_schedule() asm("GOTO PICOS_START_SCHEDULE\n")

#define exit()                          \
    do {                                \
        run_task_info |= RUN_TASK_EXIT; \
        set_timer_delay(1);             \
        while (1)                       \
            ;                           \
    } while (0)

typedef union {
    struct {
        unsigned in : 2;
        unsigned out : 2;
        unsigned cnt : 4;
    };
} wait_cnt_t;

extern uint8_t int0_queue;
extern wait_cnt_t int0_cnt;

extern uint8_t int1_queue;
extern wait_cnt_t int1_cnt;

extern uint8_t int2_queue;
extern wait_cnt_t int2_cnt;

extern uint8_t tx_wait;

#define get_pid() ((run_task_info >> 4) & 0x3)

#define int_wait_queue_push(int_num)                                       \
    do {                                                                   \
        lock();                                                            \
        int##int_num##_queue &= ~(0x3 << (int##int_num##_cnt.in * 2));     \
        int##int_num##_queue |=                                            \
            (((run_task_info >> 4) & 0x3) << (int##int_num##_cnt.in * 2)); \
        int##int_num##_cnt.in = (int##int_num##_cnt.in + 1) & 0x3;         \
        int##int_num##_cnt.cnt++;                                          \
        wait_task_info |= (1 << ((run_task_info >> 4) & 0x3));             \
        unlock();                                                          \
        set_timer_delay(1);                                                \
        while (wait_task_info & (1 << ((run_task_info >> 4) & 0x3)))       \
            ;                                                              \
    } while (0)

#define int_wait_queue_pop(int_num)                                        \
    if (int##int_num##_cnt.cnt) {                                          \
        int##int_num##_cnt.cnt--;                                          \
        wait_task_info ^=                                                  \
            1 << ((int##int_num##_queue >> (int##int_num##_cnt.out * 2)) & \
                  0x3);                                                    \
        int##int_num##_cnt.out = (int##int_num##_cnt.out + 1) & 0x3;       \
    }
