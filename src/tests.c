#include <tests.h>

char a[64];
uint16_t rand_seed;

static uint8_t yee_rand(void)
{
    rand_seed = (uint16_t) (rand_seed * 25173u + 13849u);
    return (uint8_t) (rand_seed >> 8);
}

void extern_memory_test(void)
{
    for (int i = 0; i < 64; i++)
        a[i] = i;
    extern_memory_write(0x5d00, a);
    for (int i = 0; i < 64; i++)
        a[i] = 0;

    extern_memory_read(0x5d00, a);
    for (int i = 0; i < 64; i++) {
        printf("%x: %x\n", i, a[i]);
    }

    rand_seed = 0xdead;
    for (uint16_t addr = 0; addr < 0x2000; addr++) {
        for (int i = 0; i < 64; i++)
            a[i] = yee_rand();
        extern_memory_write(addr, a);
    }

    for (uint16_t addr = 0x4000; addr < 0x6000; addr++) {
        for (int i = 0; i < 64; i++)
            a[i] = yee_rand();
        extern_memory_write(addr, a);
    }

    for (int i = 0; i < 64; i++)
        a[i] = 0;
    rand_seed = 0xdead;
    int fail = 0;

    for (uint16_t addr = 0; addr < 0x2000; addr++) {
        extern_memory_read(addr, a);
        if ((addr & 0xFF) == 0) {
            printf("addr: 0x%x\n", addr);
        }
        for (int i = 0; i < 64; i++) {
            if (a[i] != yee_rand()) {
                fail = 1;
                printf("fail: %x, addr = 0x%x\n", i, addr);
                goto done;
            }
        }
    }
    for (uint16_t addr = 0x4000; addr < 0x6000; addr++) {
        extern_memory_read(addr, a);
        if ((addr & 0xFF) == 0) {
            printf("addr: 0x%x\n", addr);
        }
        for (int i = 0; i < 64; i++) {
            if (a[i] != yee_rand()) {
                fail = 1;
                printf("fail: %x, addr = 0x%x\n", i, addr);
                goto done;
            }
        }
    }
done:
    if (!fail) {
        printf("success\n");
    }
}