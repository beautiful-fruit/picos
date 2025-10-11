#include <type.h>

#define RUN_TASK_SIZE 4
#define RUN_TASK_MASK 0xF
extern Task *current;
extern Task run_task[RUN_TASK_SIZE];
extern unsigned char run_task_info;

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