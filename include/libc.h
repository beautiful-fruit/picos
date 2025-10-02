#include <hal.h>
#include <stdarg.h>

void putchar(char c);

static void print_uint(unsigned int x);

void printf(const char *fmt, ...);

#define PANIC(fmt, ...)                                                       \
    do {                                                                      \
        printf("\n=== KERNEL PANIC ===\n");                                   \
        printf(fmt "\nLocation: %s:%d\n", ##__VA_ARGS__, __FILE__, __LINE__); \
        while (1)                                                             \
            ;                                                                 \
    } while (0)
