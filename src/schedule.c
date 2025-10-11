#include <kernel.h>
#include <libc.h>
#include <xc.h>

Task run_task[RUN_TASK_SIZE];
/* [round robin cnt 2 bits | run task use 4 bits] */
unsigned char run_task_info = 0;

Task *current = NULL;

signed char create_process(void (*func)(void))
{
    if ((run_task_info & RUN_TASK_MASK) == RUN_TASK_MASK)
        return -1;
    char i = 0;
    for (; (1 << i) & run_task_info; i++)
        ;

    run_task_info |= (1 << i);
    run_task[i].sp = run_task[i].stack;
    run_task[i].context.pc.value = (__uint24) func;

    run_task[i].context.rasp = 0;

    return 0;
}


Task *schedule()
{
    if (!(run_task_info & RUN_TASK_MASK))
        return NULL;

    char i = ((run_task_info >> 4) + 1) & 0x3;
    for (; !((1 << i) & run_task_info); i = (i + 1) & 0x3)
        ;

    run_task_info &= RUN_TASK_MASK;
    run_task_info |= (i << 4);

    return &run_task[i];
}