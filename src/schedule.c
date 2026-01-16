#include <kernel.h>
#include <libc.h>
#include <schedule.h>

stack_status_t stack_status = {0, 4, 0};

volatile unsigned char run_task_info = 0;
volatile uint8_t wait_task_info = 0;

Task *current = NULL;

unsigned char wait_in = 0;
unsigned char wait_out = 0;

uint8_t int0_queue = 0;
wait_cnt_t int0_cnt = {0};

uint8_t int1_queue = 0;
wait_cnt_t int1_cnt = {0};

uint8_t int2_queue = 0;
wait_cnt_t int2_cnt = {0};

uint8_t tx_wait = 0xFF;
uint8_t rc_wait = 0xFF;

uint8_t stack_alloc(uint8_t stack_size)
{
    if (stack_status.use == 0xF || stack_size == 0)
        return 4;
    uint8_t idx = 0, len = 0;
    for (uint8_t i = 0; i < RUN_STACK_SIZE; i++) {
        if (!((stack_status.use >> i) & 1)) {
            len++;
            if (len == stack_size) {
                for (i = idx; i < idx + len; i++)
                    stack_status.use |= (1 << i);
                return idx;
            }
        } else {
            idx = i + 1;
            len = 0;
        }
    }
    return 4;
}

void stack_release(uint8_t pid)
{
    for (uint8_t i = run_task[pid].stack_info.stack_start;
         i < run_task[pid].stack_info.stack_start +
                 run_task[pid].stack_info.stack_size;
         i++)
        stack_status.use ^= (1 << i);
}

char create_process(func_t func, uint8_t stack_size)
{
    if (stack_size > RUN_STACK_SIZE)
        return 0;
    if ((run_task_info & RUN_TASK_MASK) == RUN_TASK_MASK ||
        (stack_status.wait_pid != 4 && stack_size != 0)) {
        if (!wait_queue_full()) {
            wait_queue_in(func, stack_size);
            return 1;
        }
        return 0;
    }

    // this need compare and switch, so lock needed
    char i = 0;
    for (; (1 << i) & run_task_info; i++)
        ;

    run_task_info |= (1 << i);
    run_task[i].context.pc.value = (__uint24) func;
    run_task[i].context.rasp = 0;

    run_task[i].stack_info.stack_size = stack_size;
    run_task[i].stack_info.stack_start = stack_alloc(stack_size);

    if (stack_size != 0 && run_task[i].stack_info.stack_start == 4)
        stack_status.wait_pid = i;
    else
        run_task[i].sp = run_stack[run_task[i].stack_info.stack_start];

    return 1;
}

void __attribute__((naked)) idle_func(void)
{
    while (1)
        ;
}

void init_scheduler()
{
    idle_task.context.pc.value = (__uint24) idle_func;
    idle_task.context.rasp = 0;
}


Task *schedule()
{
    // schedule the task not wait for interrupt and stack
    if (!(((run_task_info ^ wait_task_info) ^ (1 << stack_status.wait_pid)) &
          RUN_TASK_MASK))
        return &idle_task;

    char i = ((run_task_info >> 4) + 1) & 0x3;
    for (; !((1 << i) &
             ((run_task_info ^ wait_task_info) ^ (1 << stack_status.wait_pid)));
         i = (i + 1) & 0x3)
        ;

    run_task_info &= RUN_TASK_MASK;
    run_task_info |= (i << 4);

    return &run_task[i];
}
