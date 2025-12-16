#include <ch375.h>
#include <libc.h>
#include <tests.h>
#include <dma.h>

uint8_t disk_buf[512];

int mode = -1;
int start = 0;
extern struct dma_request req;
extern uint8_t output_data;

void main(void)
{
    ADCON1 = 0xF;
    uart_init();
    dma_init();
    extern_memory_init();
    ch375_init();
    printf("hello\n");
    GIE = 1;
    while (!(usb_flags & USB_CONNECTED));
    uint8_t x = 0;
    int status;

    while (1) {
        while (1) {
            GIE = 0;
            if (mode != -1 && start)
                break;
            GIE = 1;
        }
        
        if (mode == 0) {
            status = disk_write(req.sector, req.block_addr);
        } else {
            status = disk_read(req.sector, req.block_addr);
        }
        if (status < 0)
            output_data = DMA_FLAG_FAIL;
        else
            output_data = DMA_FLAG_SUCCESS;

        mode = -1;
        start = 0;
        GIE = 1;
        dma_trigger_interrupt();
        
    }

    while (1);
    PANIC("hello\n");
}
