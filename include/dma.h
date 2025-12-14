#pragma once
#include <libc.h>

#define DMA_WE LATC5
#define DMA_OE LATB4


/**
 * DMA_MEM_LOCK - A hardware memory lock
 *
 * 0: Connect second SRAM to main board
 * 1: Connect second SRAM to DMA board
 */
#define DMA_MEM_LOCK LATC1

#define DMA_FLAG_MAGIC (0b10110000)
#define DMA_FLAG_SUCCESS (DMA_FLAG_MAGIC | (1 << 0))
#define DMA_FLAG_FAIL (DMA_FLAG_MAGIC | (1 << 1))
#define DMA_FLAG_SENDING (DMA_FLAG_MAGIC | (1 << 2))


struct dma_request {
    uint32_t sector;
    uint16_t block_addr;
    uint8_t mode;
};



void dma_init(void);

int disk_write(uint32_t sector, uint32_t addr);

int disk_read(uint32_t sector, uint32_t addr);
