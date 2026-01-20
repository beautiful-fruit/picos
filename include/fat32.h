#pragma once
#include <memory.h>
#include <type.h>

#define BLOCK_SIZE 512

#define END_OF_CLUS 0x0FFFFFF8UL

typedef struct {
    uint32_t first_fat_sec;
    uint32_t first_data_sec;
    uint32_t byte_per_sec;
    uint8_t sec_per_clus;
    uint32_t root_clus;

    uint32_t fs_info;
    uint32_t fsi_free_cnt;
    uint32_t fsi_nxt_free;
} fat32_t;

#define DIR_ATTR_READ_ONLY 0x01
#define DIR_ATTR_HIDDEN 0x02
#define DIR_ATTR_SYSTEM 0x04
#define DIR_ATTR_VOLUME_ID 0x08
#define DIR_ATTR_DIRECTORY 0x10
#define DIR_ATTR_ARCHIVE 0x20
#define DIR_ATTR_LONG_NAME                                    \
    (DIR_ATTR_READ_ONLY | DIR_ATTR_HIDDEN | DIR_ATTR_SYSTEM | \
     DIR_ATTR_VOLUME_ID)

#define DIR_ATTR_VAL_MASK 0x3F
/* below attribute should not write back to disk */
#define DIR_ATTR_DIRTY 0x40
#define DIR_ATTR_NAME_CHANGE 0x80

typedef struct __attribute__((packed)) {
    uint8_t short_name[11];
    uint8_t attr;
    uint8_t ntres;  // useless
    uint8_t crt_time_tenth;
    uint16_t crt_time;
    uint16_t crt_date;
    uint16_t lst_acc_date;
    uint16_t fst_clus_hi;
    uint16_t wrt_time;
    uint16_t wrt_date;
    uint16_t fst_clus_lo;
    uint32_t file_size;

} fat32_dir_t;

typedef struct __attribute__((packed)) {
    uint8_t ord;
    uint8_t name1[10];
    uint8_t attr;
    uint8_t type;
    uint8_t chk_sum;
    uint8_t name2[12];
    uint16_t meow;
    uint8_t name3[4];
} fat32_long_name_t;


#define LONGEST_NAME_SZ 11

typedef struct {
    uint8_t name[LONGEST_NAME_SZ + 1];  // long name, this maybe replace to
                                        // uint8_t array in picos
    uint16_t block_entry_start;
    uint16_t block_entry_end;  // entry offset in block
    uint8_t attr;
    uint32_t file_size;
    uint32_t fst_clus;
    addr_t fst_dir_block;  // if this file is dir
} file_t;

typedef struct dir_block {
    file_t file;
    addr_t next;
    uint32_t clus;
    uint32_t entry_offset;
    /*
        size of dir_block_t should close to page size
    */
} dir_block_t;

fat32_t *create_fat32();

addr_t load_dir(fat32_t *fs, uint32_t clus);

void ls_dir(addr_t dir);

void free_dir_block(addr_t dir);

addr_t find_file(addr_t dir, const char *file_name, uint8_t name_size);

void read_file(fat32_t *fs, file_t *file, addr_t read_extern_buf, uint16_t cnt);
