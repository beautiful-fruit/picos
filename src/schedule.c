#include <kernel.h>
#include <libc.h>
#include <schedule.h>

Task run_task[RUN_TASK_SIZE];

Task idle_task;

uint8_t run_stack[4][USTACK_SIZE];

volatile unsigned char run_task_info = 0;
volatile uint8_t wait_task_info = 0;

Task *current = NULL;

func_t wait_queue[WAIT_QUEUE_SIZE];
unsigned char wait_in = 0;
unsigned char wait_out = 0;

uint8_t int0_queue = 0;
wait_cnt_t int0_cnt = {0};

uint8_t int1_queue = 0;
wait_cnt_t int1_cnt = {0};

uint8_t int2_queue = 0;
wait_cnt_t int2_cnt = {0};

char create_process(func_t func)
{
    if ((run_task_info & RUN_TASK_MASK) == RUN_TASK_MASK) {
        if (!wait_queue_full()) {
            wait_queue_in(func);
            return 1;
        }
        return 0;
    }

    // this need compare and switch, so lock needed
    char i = 0;
    for (; (1 << i) & run_task_info; i++)
        ;

    run_task_info |= (1 << i);
    run_task[i].sp = run_stack[i];
    run_task[i].context.pc.value = (__uint24) func;

    run_task[i].context.rasp = 0;

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
    if (!((run_task_info ^ wait_task_info) & RUN_TASK_MASK))
        return &idle_task;

    char i = ((run_task_info >> 4) + 1) & 0x3;
    for (; !((1 << i) & (run_task_info ^ wait_task_info)); i = (i + 1) & 0x3)
        ;

    run_task_info &= RUN_TASK_MASK;
    run_task_info |= (i << 4);

    return &run_task[i];
}
