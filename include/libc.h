#pragma once

#include <hal.h>
#include <stdarg.h>
#include <type.h>

void putchar(char c);

static void print_uint(unsigned int x);

void printf(const char *fmt, ...);

#define PANIC(fmt, ...)                                                       \
    do {                                                                      \
        INTCONbits.GIE = 0;                                                   \
        printf("\n=== KERNEL PANIC ===\n");                                   \
        printf(fmt "\nLocation: %s:%d\n", ##__VA_ARGS__, __FILE__, __LINE__); \
        while (1)                                                             \
            ;                                                                 \
    } while (0)

extern uint8_t alloc_cache[64];

#define MAP_BLOCK_CNT 2

#define EXTERN_NULL (0x7FFFULL << 6)
#define DISK_MEMORY_START (1048576ULL)

void alloc_init(void);

addr_t extern_alloc(void);

void extern_release(addr_t addr);

addr_t disk_extern_alloc(void);

void disk_extern_release(addr_t addr);

void memcpy(void *dest, const void *src, uint16_t size);

#define min(a, b) ((a) <= (b) ? (a) : (b))

#define max(a, b) ((a) >= (b) ? (a) : (b))
