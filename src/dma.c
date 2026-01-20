#include <dma.h>

void dma_init(void)
{
    TRISB0 = 1;
    INT1IF = 0;
    INT1IE = 1;
    SSPEN = 0;
    TRISC5 = 0;
    TRISB4 = 0;
    DMA_WE = 1;
    DMA_OE = 1;
}

void wait_for_int1(void)
{
    while (PORTBbits.RB1)
        ;
    INT1IF = 0;
    return;
}


uint8_t dma_request(struct dma_request *req)
{
    INT1IF = 0;
    uint8_t *ptr = (uint8_t *) req;
    uint8_t sz = sizeof(struct dma_request);
    while (sz--) {
        LATD = *ptr++;
        DMA_WE = 0;
        DMA_WE = 1;
        while (PORTBbits.RB1)
            ;
        while (!PORTBbits.RB1)
            ;

        INT1IF = 0;
    }
    TRISD = 0xFF;
    DMA_OE = 0;
    while (PORTBbits.RB1)
        ;
    while (!PORTBbits.RB1)
        ;
    INT1IF = 0;
    uint8_t res;
    while (1) {
        res = PORTD;
        if ((res & 0b11111000) == DMA_FLAG_MAGIC)
            break;
    }
    DMA_OE = 1;
    TRISD = 0;
    return res;
}

int disk_write(uint32_t sector, uint32_t addr)
{
    addr -= 0x4000ULL << 6;
    struct dma_request req;
    req.sector = sector;
    req.block_addr = (uint16_t) (addr >> 9);
    req.mode = 0;
    DMA_MEM_LOCK = 1;
    INT1IF = 0;
    if (dma_request(&req) != DMA_FLAG_SUCCESS) {
        DMA_MEM_LOCK = 0;
        return -1;
    }
    while (PORTBbits.RB1)
        ;
    while (!PORTBbits.RB1)
        ;
    DMA_MEM_LOCK = 0;
    TRISD = 0xFF;
    DMA_OE = 0;
    while (PORTBbits.RB1)
        ;
    while (!PORTBbits.RB1)
        ;
    INT1IF = 0;
    uint8_t res;
    while (1) {
        res = PORTD;
        if ((res & 0b11111000) == DMA_FLAG_MAGIC)
            break;
    }
    DMA_OE = 1;
    TRISD = 0;
    if (res == DMA_FLAG_FAIL)
        return -1;
    return 0;
}

int disk_read(uint32_t sector, uint32_t addr)
{
    addr -= 0x4000ULL << 6;
    INT1IF = 0;
    struct dma_request req;
    req.sector = sector;
    req.block_addr = (uint16_t) (addr >> 9);
    req.mode = 1;
    DMA_MEM_LOCK = 1;
    if (dma_request(&req) != DMA_FLAG_SUCCESS) {
        DMA_MEM_LOCK = 0;
        return -1;
    }
    while (PORTBbits.RB1)
        ;
    while (!PORTBbits.RB1)
        ;
    INT1IF = 0;
    DMA_MEM_LOCK = 0;
    TRISD = 0xFF;
    DMA_OE = 0;
    while (PORTBbits.RB1)
        ;
    while (!PORTBbits.RB1)
        ;
    INT1IF = 0;
    while (!PORTD)
        ;
    uint8_t res = PORTD;
    DMA_OE = 1;
    TRISD = 0;
    if (res == DMA_FLAG_FAIL)
        return -1;

    return 0;
}
