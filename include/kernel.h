#include <memory.h>
#include <type.h>

#define RUN_TASK_MASK 0xF
extern Task *current;

extern stack_status_t stack_status;

#define WAIT_QUEUE_MOD 0xF
extern unsigned char wait_in;
extern unsigned char wait_out;

/* [Nop 1 bit | exit 1 bit | round robin cnt 2 bits | run task use 4 bits] */
extern volatile unsigned char run_task_info;

/* [Nop 4 bit | wait task use 4 bits] */
extern volatile uint8_t wait_task_info;

#define RUN_TASK_EXIT 0x40

#define enter_user_func()          \
    do {                           \
        *(current->sp - 3) = TOSL; \
        *(current->sp - 2) = TOSH; \
        *(current->sp - 1) = TOSU; \
        asm("POP");                \
    } while (0)


#define return_user_func()         \
    do {                           \
        asm("PUSH");               \
        TOSL = *(current->sp - 3); \
        TOSH = *(current->sp - 2); \
        TOSU = *(current->sp - 1); \
        asm("RETURN");             \
    } while (0)

#define wait_queue_full() (wait_out == ((wait_in + 1) & WAIT_QUEUE_MOD))

#define wait_queue_empty() (wait_out == wait_in)

#define wait_queue_in(func, stack_size)              \
    {                                                \
        wait_queue[wait_in].func = func;             \
        wait_queue[wait_in].stack_size = stack_size; \
        wait_in = (wait_in + 1) & WAIT_QUEUE_MOD;    \
    }

#define wait_queue_out(out)                         \
    {                                               \
        out = wait_queue[wait_out];                 \
        wait_out = (wait_out + 1) & WAIT_QUEUE_MOD; \
    }
