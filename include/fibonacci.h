#include <kernel.h>

extern uint32_t fsstart_param;

void __attribute__((naked)) fpstart(void);

void __attribute__((naked)) fpend(void);
