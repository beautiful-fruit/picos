#include <ch375.h>
#include <hal.h>
#include <dma.h>

uint8_t output_data = 0;
uint8_t now = 0;
struct dma_request req;
uint8_t *ptr = &req;
extern int mode;
extern int start;

void __interrupt() isr(void)
{
    if (INT0IF) {
        usb_handler();
        INT0IF = 0;
    } else if (INT1IF) {
        if (!(usb_flags & USB_INT_CONNECT))
            return;
        if (mode != -1) {
            output_data = DMA_FLAG_FAIL;
            ptr = &req;
            now = 0;
            goto int1_done;
        }
        TRISD = 0xFF;
        DMA_BUS_OE = 0;
        asm(
            "NOP\n"
            "NOP\n"
            "NOP\n"
            "NOP\n"
            "NOP\n"
        );
        *ptr++ = PORTD;
        DMA_BUS_OE = 1;
        TRISD = 0x00;
        now++;

        if (now == sizeof(struct dma_request)) {
            ptr = &req;
            now = 0;
            output_data = DMA_FLAG_SUCCESS;
            mode = req.mode;
            start = 0;
        } else {
            output_data = DMA_FLAG_SENDING;
        }
int1_done:
        dma_trigger_interrupt();
        INT1IF = 0;
    } else if (INT2IF) {
        if (!(usb_flags & USB_INT_CONNECT))
            return;
        if (!INTEDG2) {
            INTEDG2 = 1;
            INT2IF = 0;
            TRISD = 0;
            LATD = output_data;
            DMA_BUS_DIR = 0;
            DMA_BUS_OE = 0;
            dma_trigger_interrupt();
            while (!INT2IF);
            INTEDG2 = 0;
            DMA_BUS_OE = 1;
            DMA_BUS_DIR = 1;
            start = 1;
            printf("start\n");
        }
        INT2IF = 0;
    }
}
