#pragma once

#include <hal.h>
#include <type.h>

/* ========= kernel ========= */
#define run_task (mem._run_task)
#define idle_task (mem._idle_task)
#define run_stack (mem._run_stack)

#define wait_queue (mem._wait_queue)

#define RUN_TASK_SIZE 4
#define RUN_STACK_SIZE 4
#define WAIT_QUEUE_SIZE 16

/* ========= fat32 ========= */
#define picos_fat_cache (mem._picos_fat_cache)
#define picos_cache (mem._picos_cache)
#define dir_block_cache (mem._dir_block_cache)

struct memory {
    /* ========= kernel ========= */
    Task _run_task[RUN_TASK_SIZE];
    Task _idle_task;
    uint8_t _run_stack[RUN_STACK_SIZE][USTACK_SIZE];

    wait_node_t _wait_queue[WAIT_QUEUE_SIZE];

    /* ========= fat32 ========= */
    unsigned char _picos_fat_cache[64];
    unsigned char _picos_cache[64];
    unsigned char _dir_block_cache[64];
};

extern struct memory mem;
