extern char ustack0[256];
extern char *sp;

#define enter_user_func(stack_occupy)      \
    do {                                   \
        *(sp - stack_occupy) = TOSL;       \
        *(sp - (stack_occupy - 1)) = TOSH; \
        *(sp - (stack_occupy - 2)) = TOSU; \
        asm("POP");                        \
    } while (0)


#define return_user_func(stack_occupy)     \
    do {                                   \
        asm("PUSH");                       \
        TOSL = *(sp - stack_occupy);       \
        TOSH = *(sp - (stack_occupy - 1)); \
        TOSU = *(sp - (stack_occupy - 2)); \
        asm("RETURN");                     \
    } while (0)