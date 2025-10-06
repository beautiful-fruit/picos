extern char ustack0[256];
extern char *sp;

#define enter_user_func() \
    do {                  \
        *(sp - 3) = TOSL; \
        *(sp - 2) = TOSH; \
        *(sp - 1) = TOSU; \
        asm("POP");       \
    } while (0)


#define return_user_func() \
    do {                   \
        asm("PUSH");       \
        TOSL = *(sp - 3);  \
        TOSH = *(sp - 2);  \
        TOSU = *(sp - 1);  \
        asm("RETURN");     \
    } while (0)