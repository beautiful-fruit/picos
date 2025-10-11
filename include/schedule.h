#pragma once
#include <kernel.h>

/**
 * create_process - create a new process
 * @func: The function that the process will execute
 *
 * Return 0 if success, return -1 if fail.
 */
signed char create_process(void (*func)(void));

Task *schedule();

#define start_schedule() asm("GOTO PICOS_START_SCHEDULE\n")