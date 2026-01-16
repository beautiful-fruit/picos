#include <hal.h>
#include <stdarg.h>
#include <type.h>

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

extern uint8_t alloc_cache[64];

#define MAP_BLOCK_CNT 2

#define EXTERN_NULL 0x7FFFULL << 6

#define alloc_init()                                                \
    do {                                                            \
        for (uint8_t _i = 0; _i < 64; _i++)                         \
            alloc_cache[_i] = 0;                                    \
        for (uint8_t _i = 1; _i < MAP_BLOCK_CNT; _i++) {            \
            extern_memory_write(_i, (char *) alloc_cache);          \
            extern_memory_write(_i + 0x4000, (char *) alloc_cache); \
        }                                                           \
        alloc_cache[0] = 0x01;                                      \
        extern_memory_write(0, (char *) alloc_cache);               \
        extern_memory_write(0x4000, (char *) alloc_cache);          \
        extern_memory_read(0x4000, (char *) alloc_cache);           \
    } while (0)

/**
 * @block_num: uint16_t
 * @addr: addr_t, the address of extern memory
 */
#define extern_alloc(block_num, addr)                                         \
    do {                                                                      \
        addr = EXTERN_NULL;                                                   \
        if (block_num <= 8) {                                                 \
            for (uint8_t _i = 0;                                              \
                 _i < MAP_BLOCK_CNT * 64 && addr == EXTERN_NULL; _i++) {      \
                extern_memory_read((_i >> 6), (char *) alloc_cache);          \
                for (uint8_t _j = 0; _j < 8 && addr == EXTERN_NULL; _j++) {   \
                    if (!((1 << _j) & alloc_cache[_i & 0x3F])) {              \
                        alloc_cache[_i & 0x3F] |= 1 << _j;                    \
                        addr = ((((addr_t) _i) << 3) + ((addr_t) _j)) << 9;   \
                        extern_memory_write((_i >> 6), (char *) alloc_cache); \
                    }                                                         \
                }                                                             \
            }                                                                 \
        }                                                                     \
    } while (0)

/**
 * @addr: addr_t, the address of extern memory
 */
#define extern_release(addr)                                          \
    do {                                                              \
        extern_memory_read((addr >> 18), (char *) alloc_cache);       \
        alloc_cache[(addr >> 12) & 0x3F] ^= 1 << ((addr >> 9) & 0x7); \
        extern_memory_write((addr >> 18), (char *) alloc_cache);      \
    } while (0)

/**
 * @block_num: uint16_t
 * @addr: addr_t, the address of extern memory
 */
#define disk_extern_alloc(block_num, addr)                                     \
    do {                                                                       \
        addr = EXTERN_NULL;                                                    \
        if (block_num <= 8) {                                                  \
            for (uint8_t _i = 0;                                               \
                 _i < MAP_BLOCK_CNT * 64 && addr == EXTERN_NULL; _i++) {       \
                extern_memory_read((_i >> 6) + 0x4000UL,                       \
                                   (char *) alloc_cache);                      \
                for (uint8_t _j = 0; _j < 8 && addr == EXTERN_NULL; _j++) {    \
                    if (!((1 << _j) & alloc_cache[_i & 0x3F])) {               \
                        alloc_cache[_i & 0x3F] |= 1 << _j;                     \
                        addr = (((((addr_t) _i) << 3) + ((addr_t) _j)) << 9) + \
                               (addr_t) 1048576ULL;                            \
                        extern_memory_write((_i >> 6) + 0x4000UL,              \
                                            (char *) alloc_cache);             \
                    }                                                          \
                }                                                              \
            }                                                                  \
        }                                                                      \
    } while (0)
/**
 * @addr: addr_t, the address of extern memory
 */
#define disk_extern_release(addr)                                           \
    do {                                                                    \
        addr -= (addr_t) 1048576ULL;                                        \
        extern_memory_read((addr >> 18) + 0x4000UL, (char *) alloc_cache);  \
        alloc_cache[(addr >> 12) & 0x3F] ^= 1 << ((addr >> 9) & 0x7);       \
        extern_memory_write((addr >> 18) + 0x4000UL, (char *) alloc_cache); \
    } while (0)

int memcmp(const char *x, const char *y, uint16_t size);

#define min(a, b) ((a) <= (b) ? (a) : (b))

#define max(a, b) ((a) >= (b) ? (a) : (b))
