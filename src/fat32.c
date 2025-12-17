#include <dma.h>
#include <fat32.h>
#include <hal.h>
#include <libc.h>

fat32_t __fs;

unsigned char picos_fat_cache[64];
unsigned char picos_cache[64];
unsigned char dir_block_cache[64];

#define is_valid_ascii(c) (c >= 0x20 && c <= 0x7E)

#define get_clus_first_sec(fs, n) \
    ((n - 2) * (fs)->sec_per_clus + (fs)->first_data_sec)

#define get_clus_fat_sec(fs, n) \
    ((fs)->first_fat_sec + (n * 4) / (fs)->byte_per_sec)

#define get_clus_fat_offset(fs, n) ((n * 4) % (fs)->byte_per_sec)

#define get_next_clus(fs, clus, fat_buf) \
    (*((uint32_t *) (fat_buf + get_clus_fat_offset(fs, clus))) & 0x0FFFFFFF)

#define set_next_clus(fs, clus, fat_buf, value)                      \
    do {                                                             \
        *((uint32_t *) (fat_buf + get_clus_fat_offset(fs, clus))) &= \
            0xF0000000;                                              \
        *((uint32_t *) (fat_buf + get_clus_fat_offset(fs, clus))) |= \
            (value & 0x0FFFFFFF);                                    \
    } while (0)


fat32_t *create_fat32()
{
    fat32_t *fs = NULL;
    addr_t sec_buf;
    disk_extern_alloc(8, sec_buf);

    while (disk_read(0, sec_buf) < 0)
        ;

    extern_memory_read((sec_buf + (512 - 64)) >> 6, picos_cache);
    uint16_t boot_sec_sig = *((uint16_t *) (picos_cache + 62));
    printf("boot sec sig: %x\n", boot_sec_sig);

    extern_memory_read(sec_buf >> 6, picos_cache);
    uint32_t fat_sz32 = *((uint32_t *) (picos_cache + 36));
    printf("fat sz32: %x\n", fat_sz32);
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            printf("%x ", picos_cache[i * 8 + j]);
        }
        printf("\n");
    }


    if (boot_sec_sig != 0xAA55 || fat_sz32 == 0)
        goto NOT_FAT32;

    fs = &__fs;
    fs->byte_per_sec = *((uint16_t *) (picos_cache + 11));
    fs->sec_per_clus = picos_cache[13];
    fs->first_fat_sec = *((uint16_t *) (picos_cache + 14));
    uint8_t num_fats = picos_cache[16];

    fs->first_data_sec = fs->first_fat_sec + num_fats * fat_sz32;

    fs->root_clus = *((uint32_t *) (picos_cache + 44));
    fs->fs_info = *((uint16_t *) (picos_cache + 48));


    disk_read(fs->fs_info, sec_buf);

    extern_memory_read((sec_buf + 448) >> 6, picos_cache);

    fs->fsi_free_cnt = *((uint32_t *) (picos_cache + 40));
    fs->fsi_nxt_free = *((uint32_t *) (picos_cache + 44));

NOT_FAT32:
    disk_extern_release(sec_buf);
    return fs;
}

addr_t load_dir(fat32_t *fs, uint32_t clus)
{
    addr_t dir_buf, fat_buf;
    disk_extern_alloc(8, dir_buf);
    fat32_dir_t *dir_cache = (fat32_dir_t *) picos_cache;
    disk_extern_alloc(8, fat_buf);
    uint32_t fat_sec = 0;

    /* create new dir_block */
    addr_t now_addr;
    extern_alloc(1, now_addr);
    // extern_memory_read(now_addr >> 6, (char *) dir_block_cache);
    dir_block_t *now = (dir_block_t *) dir_block_cache;
    now->entry_offset = 0;
    now->clus = clus;

    addr_t head = now_addr;
    addr_t last_blk;
    printf("head: %lx\n", head);
    uint32_t entry_offset = 0;

    while (1) {
        uint32_t first_sec = get_clus_first_sec(fs, clus);
        for (uint8_t i = 0; i < fs->sec_per_clus; i++) {
            disk_read(first_sec + i, dir_buf);
            uint16_t block_entry_start = 65535;
            for (uint8_t j = 0; j < BLOCK_SIZE / sizeof(fat32_dir_t); j += 2) {
                extern_memory_read((dir_buf >> 6) + (j >> 1), picos_cache);

                for (uint8_t k = 0; k < 2; k++, entry_offset++) {
                    printf("now_addr: %lx\n", now_addr);
                    printf("%s\n", dir_cache[k].short_name);
                    if (dir_cache[k].short_name[0] == 0x00)
                        goto ls_end;
                    if (dir_cache[k].short_name[0] == 0xE5)
                        continue;
                    if (dir_cache[k].attr == DIR_ATTR_LONG_NAME) {
                        fat32_long_name_t *long_name =
                            (fat32_long_name_t *) (dir_cache + k);
                        uint8_t x = 0;

                        for (uint8_t name_idx = 0; name_idx < 10; name_idx++) {
                            if (is_valid_ascii(long_name->name1[name_idx])) {
                                now->file.name[x] = long_name->name1[name_idx];
                                printf("%c\n", now->file.name[x]);
                                x++;
                            }
                        }
                        for (uint8_t name_idx = 0; name_idx < 12; name_idx++) {
                            if (is_valid_ascii(long_name->name2[name_idx])) {
                                now->file.name[x] = long_name->name2[name_idx];
                                printf("%c\n", now->file.name[x]);
                                x++;
                            }
                        }
                        now->file.name[x] = 0xFF;


                        if (block_entry_start == 65535)
                            block_entry_start =
                                entry_offset - now->entry_offset;
                    }
                    if (dir_cache[k].attr & (DIR_ATTR_VOLUME_ID))
                        continue;
                    now->file.block_entry_start = block_entry_start;
                    block_entry_start = 65535;
                    now->file.block_entry_end =
                        entry_offset - now->entry_offset;
                    now->file.file_size = dir_cache[k].file_size;
                    now->file.fst_clus =
                        (((uint32_t) dir_cache[k].fst_clus_hi) << 16) |
                        (uint32_t) dir_cache[k].fst_clus_lo;
                    now->file.attr = dir_cache[k].attr;
                    now->clus = clus;

                    addr_t new_block;  // this is gerneral memory region
                    extern_alloc(1, new_block);
                    printf("alloc: %lx\n", new_block);
                    now->next = new_block;

                    extern_memory_write(now_addr >> 6, (char *) now);

                    // extern_memory_read(new_block >> 6, (char *)now);
                    last_blk = now_addr;
                    now_addr = new_block;
                    now->entry_offset = entry_offset;
                    now->clus = clus;
                }
            }
        }
        if (get_clus_fat_sec(fs, clus) != fat_sec) {
            fat_sec = get_clus_fat_sec(fs, clus);
            disk_read(fat_sec, fat_buf);
        }

        uint32_t clus_offset = (clus * 4) % (fs)->byte_per_sec;  // can expand

        extern_memory_read(
            (fat_buf + clus_offset) >> 6,
            (char *) picos_fat_cache);  // maybe can reduce memory read


        clus =
            *((uint32_t *) (picos_fat_cache + (clus_offset % 64))) & 0x0FFFFFFF;
    }
ls_end:
    extern_release(dir_buf);
    extern_release(fat_buf);
    extern_release(now_addr);
    now_addr = last_blk;
    extern_memory_read(now_addr >> 6, (char *) now);
    now->next = EXTERN_NULL;
    extern_memory_write(now_addr >> 6, (char *) now);

    return head;
}

#define print_file_name(j, file)                                            \
    do {                                                                    \
        for (uint16_t j = 0; j < LONGEST_NAME_SZ && (file).name[j] != 0xFF; \
             j++)                                                           \
            if (is_valid_ascii((file).name[j]))                             \
                printf("%c", (file).name[j]);                               \
        printf("\n");                                                       \
    } while (0)



void ls_dir(addr_t dir)
{
    while (1) {
        if (dir == EXTERN_NULL)
            break;
        extern_memory_read(dir >> 6, dir_block_cache);
        print_file_name(j, ((dir_block_t *) dir_block_cache)->file);
        dir = (addr_t) (((dir_block_t *) dir_block_cache)->next);
    }
}

void free_dir_block(addr_t dir)
{
    while (1) {
        if (dir == EXTERN_NULL)
            break;
        extern_memory_read(dir >> 6, (char *) dir_block_cache);
        extern_release(dir);
        dir = (addr_t) (((dir_block_t *) dir_block_cache)->next);
    }
}

addr_t find_file(addr_t dir, const char *file_name, uint8_t name_size)
{
    addr_t target = 0;
    while (1) {
        if (dir == EXTERN_NULL)
            break;
        extern_memory_read(dir >> 6, dir_block_cache);
        int result = memcmp(((dir_block_t *) dir_block_cache)->file.name,
                            file_name, name_size);
        if (!result) {
            target = dir;
            goto find_file;
        }
        dir = (addr_t) (((dir_block_t *) dir_block_cache)->next);
    }

find_file:
    return target;
}

void read_file(fat32_t *fs, file_t *file, addr_t read_extern_buf, size_t cnt)
{
    if (file->file_size == 0)
        return;
    if ((cnt & 0x1ff) != 0)
        return;

    addr_t fat_extern_buf, tmp_extern_buf;
    disk_extern_alloc(BLOCK_SIZE >> 6, fat_extern_buf);
    disk_extern_alloc(BLOCK_SIZE >> 6, tmp_extern_buf);

    uint32_t clus = file->fst_clus;
    uint32_t fat_sec = 0;

    while (clus < END_OF_CLUS) {
        uint32_t first_sec = get_clus_first_sec(fs, clus);
        
        for (uint8_t i = 0;i < fs->sec_per_clus;i++) {
            disk_read(first_sec + i, tmp_extern_buf);
            for (uint16_t j = 0;j < BLOCK_SIZE;j += 64, read_extern_buf += 64) {
                extern_memory_read((tmp_extern_buf + j) >> 6, picos_cache);
                extern_memory_write(read_extern_buf >> 6, picos_cache);
            }
            cnt -= BLOCK_SIZE;
            if (cnt == 0) 
                goto end_read;
        }
        
        if (((fs)->first_fat_sec + (clus * 4) / (fs)->byte_per_sec) != fat_sec) {
            fat_sec = ((fs)->first_fat_sec + (clus * 4) / (fs)->byte_per_sec);
            disk_read(fat_sec, fat_extern_buf);
        }

        extern_memory_read((fat_extern_buf + ((((clus * 4) % (fs)->byte_per_sec) / 64) * 64)) >> 6, picos_fat_cache); //maybe can reduce memory read

        clus = *((uint32_t *)(picos_fat_cache + (((clus * 4) % (fs)->byte_per_sec) % 64))) & 0x0FFFFFFF;

    }
end_read:
    disk_extern_release(tmp_extern_buf);
    disk_extern_release(fat_extern_buf);
    return;
}
