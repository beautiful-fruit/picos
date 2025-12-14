#include <dma.h>

void dma_init(void)
{
    IPEN = 0;
    TRISB1 = 1;
    TRISB2 = 1;
    INT1IF = 0;
    INT1IE = 1;
    INT2IF = 0;
    INT2IE = 1;
    INTEDG1 = 0;
    INTEDG2 = 0;
    PEIE = 1;
    TRISC1 = 0;
    TRISB3 = 0;
    TRISB4 = 0;
    DMA_BUS_DIR = 1;
    DMA_BUS_OE = 1;
    DMA_INT_TRIGGER = 1;
    
    printf("dma init\n");
}

void dma_trigger_interrupt(void)
{
    DMA_INT_TRIGGER = 0;
    asm(
        "NOP\n"
        "NOP\n"
        "NOP\n"
        "NOP\n"
        "NOP\n"
        "NOP\n"
        "NOP\n"
        "NOP\n"
        "NOP\n"
        "NOP\n"
    );
    DMA_INT_TRIGGER = 1;
}