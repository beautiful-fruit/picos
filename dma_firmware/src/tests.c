#include <tests.h>

// char a[256];
uint16_t rand_seed;

static uint8_t yee_rand(void)
{
    rand_seed = (uint16_t) (rand_seed * 25173u + 13849u);
    return (uint8_t) (rand_seed >> 8);
}

void extern_memory_test(void)
{
    
    // for (uint16_t i = 0; i < 64; i++) {
    //     extern_memory_write(i, (uint8_t)i);
    // }
    // for (uint16_t i = 0; i < 64; i++) {
    //     a[i] = extern_memory_read(i);
    // }
    // for (uint16_t i = 0; i < 64; i++) {
    //     printf("%x: %x\n", i, a[i]);
    // }
//     for (int i = 0; i < 64; i++)
//         a[i] = i;
//     extern_memory_write(0x4000, a);
//     for (int i = 0; i < 64; i++)
//         a[i] = 0;
//     for (int i = 0; i < 1; i++) {
//         extern_memory_read(0x4000, a);
//         for (int i = 0; i < 64; i++)
//             printf("%x: %x\n", i, a[i]);
//     }
    // while(1);
    int fail = 0;
    rand_seed = 0xdead;
    for (uint32_t addr = 0; addr <= (((uint32_t)1) << 19) - 1; addr++)
        extern_memory_write(addr, yee_rand());
    rand_seed = 0xdead;
    for (uint32_t addr = 0; addr <= (((uint32_t)1) << 19) - 1; addr++) {
        uint8_t x = extern_memory_read(addr);
        if ((addr & 0xFF) == 0) {
            printf("addr: 0x%x\n", addr);
        }
        if (x != yee_rand()) {
            fail = 1;
            printf("fail: addr = 0x%x\n", addr);
            goto done;
        }
    }
//     for (uint16_t addr = 0b0; addr <= 0b101111111111111; addr++) {
//         for (int i = 0; i < 64; i++)
//             a[i] = yee_rand();
//         extern_memory_write(addr, a);
//     }

//     for (int i = 0; i < 64; i++)
//         a[i] = 0;
//     rand_seed = 0xdead;
//     

//     for (uint16_t addr = 0b0; addr <= 0b101111111111111; addr++) {
//         extern_memory_read(addr, a);
//         if ((addr & 0xFF) == 0) {
//             printf("addr: 0x%x\n", addr);
//         }
//         for (int i = 0; i < 64; i++) {
//             if (a[i] != yee_rand()) {
//                 fail = 1;
//                 printf("fail: %x, addr = 0x%x\n", i, addr);
//                 goto done;
//             }
//         }
//     }
done:
    if (!fail) {
        printf("success\n");
    }
}


void disk_test(void)
{
    int fail = 0;
    rand_seed = 0xdead;
    for (uint32_t addr = 0; addr < (((uint32_t) 32) << 20); addr += 512) {
        if ((addr & 0xFFFF) == 0)
                printf("writing addr: 0x%lx\n", addr);
        for (uint32_t i = addr; i < addr + 512; i++) {
            extern_memory_write(i & 0b1111111111111111111, yee_rand());
        }
        if (disk_write(addr >> 9, (addr & 0b1111111111111111111) >> 9)) {
            fail = 1;
            printf("disk_write fail: addr = %lx\n", addr);
            goto disk_test_done;
        }
    }

    rand_seed = 0xdead;
    for (uint32_t addr = 0; addr < (((uint32_t) 32) << 20); addr += 512) {
        if (disk_read(addr >> 9, (addr & 0b1111111111111111111) >> 9)) {
            fail = 1;
            printf("disk_read fail: addr = %lx\n", addr);
            goto disk_test_done;
        }

        for (uint32_t i = addr; i < addr + 512; i++) {
            if ((i & 0xFFFF) == 0)
                printf("addr: 0x%lx\n", addr);
            if (extern_memory_read(i & 0b1111111111111111111) != yee_rand()) {
                printf("disk_read value fail: addr = %lx\n", i);
                fail = 1;
                goto disk_test_done;
            }
        }
    }
    
disk_test_done:
    if (!fail) {
        printf("success\n");
    }
}