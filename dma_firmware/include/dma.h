#include <libc.h>

#define DMA_BUS_OE LATB3
#define DMA_BUS_DIR LATB4 // 1 for input, 0 for output
#define DMA_INT_TRIGGER LATC1 // 0 for interrupt

#define DMA_FLAG_MAGIC (0b10110000)
#define DMA_FLAG_SUCCESS (DMA_FLAG_MAGIC | (1 << 0))
#define DMA_FLAG_FAIL (DMA_FLAG_MAGIC | (1 << 1))
#define DMA_FLAG_SENDING (DMA_FLAG_MAGIC | (1 << 2))
#define DMA_FLAG_NRESET (DMA_FLAG_MAGIC | (1 << 3))

struct dma_request {
    uint32_t sector;
    uint16_t block_addr;
    uint8_t mode;
};


void dma_init(void);

void dma_trigger_interrupt(void);
