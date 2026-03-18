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

static void alloc_init()
{
    uint8_t i;
    for (i = 0; i < 64; i++)
        alloc_cache[i] = 0;
    for (i = 1; i < MAP_BLOCK_CNT; i++) {
        extern_memory_write(i, (char *) alloc_cache);
        extern_memory_write(i + DISK_MEMORY_START, (char *) alloc_cache);
    }
    /* The first 8 block use as bit map */
    alloc_cache[0] = 0x01;
    extern_memory_write(0, (char *) alloc_cache);
    extern_memory_write(DISK_MEMORY_START, (char *) alloc_cache);
}

static addr_t extern_alloc()
{
    addr_t addr = EXTERN_NULL;
    for (uint8_t i = 0; i < MAP_BLOCK_CNT; i++) {
        extern_memory_read(i, (char *) alloc_cache);

        for (uint8_t j = 0; j < 64; j++) {
            if (alloc_cache[j] == 0xFF)
                continue;
            for (uint8_t k = 0; k < 8; k++) {
                if (!((1 << k) & alloc_cache[j])) {
                    alloc_cache[j] |= 1 << k;
                    addr = ((((((addr_t) i) << 6) + ((addr_t) j)) << 3) + k)
                           << 9;
                    extern_memory_write(i, (char *) alloc_cache);
                    goto extern_alloc_end;
                }
            }
        }
    }
extern_alloc_end:
    return addr;
}

static void extern_release(addr_t addr)
{
    extern_memory_read((addr >> 18), (char *) alloc_cache);
    alloc_cache[(addr >> 12) & 0x3F] &= 0xFF ^ (1 << ((addr >> 9) & 0x7));
    extern_memory_write((addr >> 18), (char *) alloc_cache);
}

static addr_t disk_extern_alloc()
{
    addr_t addr = EXTERN_NULL;
    for (uint8_t i = 0; i < MAP_BLOCK_CNT; i++) {
        extern_memory_read(i + DISK_MEMORY_START, (char *) alloc_cache);

        for (uint8_t j = 0; j < 64; j++) {
            if (alloc_cache[j] == 0xFF)
                continue;
            for (uint8_t k = 0; k < 8; k++) {
                if (!((1 << k) & alloc_cache[j])) {
                    alloc_cache[j] |= 1 << k;
                    addr = ((((((addr_t) i) << 6) + ((addr_t) j)) << 3) + k)
                           << 9;
                    extern_memory_write(i + DISK_MEMORY_START,
                                        (char *) alloc_cache);
                    goto disk_extern_alloc_end;
                }
            }
        }
    }
disk_extern_alloc_end:
    return addr + DISK_MEMORY_START;
}

static void disk_extern_release(addr_t addr)
{
    addr -= DISK_MEMORY_START;
    extern_memory_read((addr >> 18) + DISK_MEMORY_START, (char *) alloc_cache);
    alloc_cache[(addr >> 12) & 0x3F] &= 0xFF ^ (1 << ((addr >> 9) & 0x7));
    extern_memory_write((addr >> 18) + DISK_MEMORY_START, (char *) alloc_cache);
}

int memcmp(const char *x, const char *y, uint16_t size);

#define min(a, b) ((a) <= (b) ? (a) : (b))

#define max(a, b) ((a) >= (b) ? (a) : (b))
