#include <kernel.h>
#include <libc.h>
#include <xc.h>

Task run_task[RUN_TASK_SIZE];
volatile unsigned char run_task_info = 0;

Task *current = NULL;

func_t wait_queue[WAIT_QUEUE_SIZE];
unsigned char wait_in = 0;
unsigned char wait_out = 0;

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
    run_task[i].sp = run_task[i].stack;
    run_task[i].context.pc.value = (__uint24) func;

    run_task[i].context.rasp = 0;

    return 1;
}


Task *schedule()
{
    if (!(run_task_info & RUN_TASK_MASK))
        PANIC("IDLE\n");

    char i = ((run_task_info >> 4) + 1) & 0x3;
    for (; !((1 << i) & run_task_info); i = (i + 1) & 0x3)
        ;

    run_task_info &= RUN_TASK_MASK;
    run_task_info |= (i << 4);

    return &run_task[i];
}