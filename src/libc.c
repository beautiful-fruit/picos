#include <libc.h>

uint8_t alloc_cache[64];

#pragma interrupt_level 1
#pragma interrupt_level 2
void putchar(char c)
{
    if (c == '\n')
        uart_putchar('\r');
    if (c == '\b') {
        uart_putchar('\b');
        uart_putchar(' ');
    }
    uart_putchar(c);
}

#pragma interrupt_level 1
#pragma interrupt_level 2
static void print_uint(unsigned int x)
{
    if (x == 0) {
        putchar('0');
        return;
    }
    char tmp[10];
    int i = 0;
    tmp[0] = '0';
    while (x) {
        tmp[i++] = x % 10 + '0';
        x /= 10;
    }
    i--;
    while (i >= 0)
        putchar(tmp[i--]);
}

#pragma interrupt_level 1
#pragma interrupt_level 2
void printf(const char *fmt, ...)
{
    va_list vargs;
    va_start(vargs, fmt);
    for (; *fmt; fmt++) {
        if (*fmt != '%') {
            putchar(*fmt);
            continue;
        }
        fmt++;
        switch (*fmt) {
        case '\0':
            goto DONE;
        case 's': {
            const char *s = va_arg(vargs, const char *);
            while (*s)
                putchar(*s++);
            break;
        }
        case 'c': {
            char c = va_arg(vargs, char);
            putchar(c);
            break;
        }
        case 'd': {
            int x = va_arg(vargs, int);
            if (x < 0) {
                putchar('-');
                x = -x;
            }
            print_uint((unsigned int) x);
            break;
        }
        case 'u': {
            unsigned int y = va_arg(vargs, unsigned int);
            print_uint(y);
            break;
        }
        case 'x': {
            unsigned int z = va_arg(vargs, unsigned int);
            for (int i = 7; i >= 0; i--) {
                putchar("0123456789abcdef"[(z >> (i << 2)) & 0xf]);
            }
            break;
        }
        case 'l': {
            fmt++;
            unsigned long z = va_arg(vargs, unsigned long);
            for (int i = 15; i >= 0; i--) {
                putchar("0123456789abcdef"[(z >> (i << 2)) & 0xf]);
            }
            break;
        }
        }
    }
DONE:
    va_end(vargs);
}

#pragma interrupt_level 1
#pragma interrupt_level 2
void memcpy(void *dest, const void *src, uint16_t size)
{
    for (uint16_t i = 0; i < size; i++)
        ((char *) dest)[i] = ((char *) src)[i];
}

#pragma interrupt_level 1
#pragma interrupt_level 2
void alloc_init()
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

#pragma interrupt_level 1
#pragma interrupt_level 2
addr_t extern_alloc()
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

#pragma interrupt_level 1
#pragma interrupt_level 2
void extern_release(addr_t addr)
{
    extern_memory_read((addr >> 18), (char *) alloc_cache);
    alloc_cache[(addr >> 12) & 0x3F] &= 0xFF ^ (1 << ((addr >> 9) & 0x7));
    extern_memory_write((addr >> 18), (char *) alloc_cache);
}

#pragma interrupt_level 1
#pragma interrupt_level 2
addr_t disk_extern_alloc()
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

#pragma interrupt_level 1
#pragma interrupt_level 2
void disk_extern_release(addr_t addr)
{
    addr -= DISK_MEMORY_START;
    extern_memory_read((addr >> 18) + DISK_MEMORY_START, (char *) alloc_cache);
    alloc_cache[(addr >> 12) & 0x3F] &= 0xFF ^ (1 << ((addr >> 9) & 0x7));
    extern_memory_write((addr >> 18) + DISK_MEMORY_START, (char *) alloc_cache);
}
