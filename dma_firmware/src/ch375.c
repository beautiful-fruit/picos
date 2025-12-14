#include <ch375.h>

struct usb_device_descriptor device_descriptor;
struct usb_config_descriptor config_descriptor;
struct usb_interface_descriptor interface_descriptor;
struct usb_hid_descriptor hid_descriptor;
struct usb_endpoint_descriptor ep_in, ep_out;

inline void CH375_WRITE(uint8_t data)
{
    TRISD = 0;
    LATD = data;
    CH375_CE = 0;
    CH375_WR = 0;
    asm("NOP");
    CH375_WR = 1;
    CH375_CE = 1;
}

inline uint8_t CH375_READ()
{
    CH375_A0 = 0;
    TRISD = 0xFF;
    CH375_CE = 0;
    CH375_RD = 0;
    asm("NOP");
    uint8_t x = PORTD;
    CH375_RD = 1;
    CH375_CE = 1;
    return x;
}

static void wait_for_interrupt()
{
    while (PORTBbits.RB0)
        ;
    return;
}


uint8_t usb_flags = 0;
uint8_t buf[8];
uint8_t last_key;
uint32_t nsectors;
uint32_t secter_size;

void usb_handler()
{
    if (!(usb_flags & USB_CONNECTED)) {
        printf("[+] USB Event\n");
        CH375_CMD(GET_STATUS);
        if (CH375_READ() != USB_INT_CONNECT) {
            printf("[!] Connection Fail\n");
            return;
        }

        printf("[+] Send Reset Signal\n");
        set_usb_mode(USB_MODE_RESET);
        set_usb_mode(USB_MODE_SOF);
        wait_for_interrupt();
        CH375_CMD(GET_STATUS);
        if (CH375_READ() != USB_INT_CONNECT) {
            printf("[!] Connection Fail\n");
            return;
        }
        printf("[+] USB Device Connected\n");


        CH375_CMD(SET_USB_SPEED);
        CH375_WRITE_DATA(USB_SPEED_FULL_SPEED);

        CH375_CMD(GET_DESCR);
        CH375_WRITE_DATA(0x01);
        wait_for_interrupt();

        CH375_CMD(GET_STATUS);
        if (CH375_READ() != USB_INT_SUCCESS) {
            CH375_CMD(SET_USB_SPEED);
            CH375_WRITE_DATA(USB_SPEED_LOW_SPEED);

            CH375_CMD(GET_DESCR);
            CH375_WRITE_DATA(0x01);
            wait_for_interrupt();

            CH375_CMD(GET_STATUS);
            if (CH375_READ() != USB_INT_SUCCESS) {
                printf("[!] Get Device Descriptor Fail\n");
                return;
            }
        }
        printf("[+] Got Device Descriptor\n");

        CH375_CMD(RD_USB_DATA);
        uint8_t len = CH375_READ();
        if (len == 18) {
            uint8_t *ptr = (uint8_t *) &device_descriptor;
            for (uint8_t i = 0; i < len; i++)
                *ptr++ = CH375_READ();
        }
#ifdef DEBUG
        print_device_descriptor(&device_descriptor);
#endif

        CH375_CMD(SET_ADDRESS);
        CH375_WRITE_DATA(0x02);
        wait_for_interrupt();
        CH375_CMD(GET_STATUS);
        if (CH375_READ() != USB_INT_SUCCESS) {
            printf("[!] Set Address Fail\n");
            return;
        }
        printf("[+] Set Address Success\n");

        CH375_CMD(SET_USB_ADDR);
        CH375_WRITE_DATA(0x02);
        
        uint8_t *ptr;
        CH375_CMD(DISK_INIT);
        wait_for_interrupt();
        if (CH375_READ() != USB_INT_SUCCESS) {
            printf("[!] Disk Initialization fail\n");
            return;
        }

        CH375_CMD(DISK_SIZE);
        wait_for_interrupt();
        if (CH375_READ() != USB_INT_SUCCESS) {
            printf("[!] Get Disk Size fail\n");
            return;
        }
        CH375_CMD(RD_USB_DATA);
        len = CH375_READ();

        ptr = ((uint8_t *) &nsectors) + 3;
        for (int i = 0; i < 4; i++)
            *ptr-- = CH375_READ();
        
        ptr = ((uint8_t *)&secter_size) + 3;
        for (int i = 0; i < 4; i++)
            *ptr-- = CH375_READ();
        
        printf("nsectors: %x\n", (uint16_t)(nsectors >> 16));
        printf("sector size: %x\n", (uint16_t)(secter_size & 0xFFFF));
        usb_flags |= USB_CONNECTED;
    }
}

int disk_write(uint32_t sector, uint16_t block_addr)
{
    if (!(usb_flags & USB_CONNECTED)) {
        printf("[!] disk_write: USB not connected\n");
        return -1;
    }
    if (sector >= nsectors) {
        printf("[!] disk_write: Invalid sector number\n");
        return -1;
    }
    uint32_t addr = ((uint32_t) block_addr) << 9;
    CH375_CMD(DISK_WRITE);
    uint8_t *ptr = (uint8_t *)&sector;
    for (int i = 0; i < 4; i++)
        CH375_WRITE_DATA(*ptr++);
    CH375_WRITE_DATA(0x01);
    wait_for_interrupt();

    for (int i = 0; i < 512; i += 64) {
        CH375_A0 = 1;
        CH375_WRITE(GET_STATUS);
        __delay_us(1);
        if (CH375_READ() != USB_INT_DISK_WRITE) {
            printf("[!] disk_write: Write error\n");
            return -1;
        }
        CH375_A0 = 1;
        CH375_WRITE(WR_USB_DATA7);
        __delay_us(1);
        CH375_WRITE_DATA(0x40);
        for (int j = i; j < i + 64; j++)
            CH375_WRITE_DATA(extern_memory_read(addr + j));

        CH375_A0 = 1;
        CH375_WRITE(DISK_WR_GO);
        __delay_us(1);
        wait_for_interrupt();
    }

    CH375_CMD(GET_STATUS);
    if (CH375_READ() != USB_INT_SUCCESS)
        return -1;

    return 0;
}

int disk_read(uint32_t sector, uint16_t block_addr)
{
    if (!(usb_flags & USB_CONNECTED)) {
        printf("[!] disk_read: USB not connected\n");
        return -1;
    }
    if (sector >= nsectors) {
        printf("[!] disk_read: Invalid sector number\n");
        return -1;
    }

    uint32_t addr = ((uint32_t) block_addr) << 9;
    CH375_CMD(DISK_READ);
    uint8_t *ptr = (uint8_t *)&sector;
    for (int i = 0; i < 4; i++)
        CH375_WRITE_DATA(*ptr++);
    CH375_WRITE_DATA(0x01);
    wait_for_interrupt();

    for (int i = 0; i < 512; i += 64) {
        CH375_A0 = 1;
        CH375_WRITE(GET_STATUS);
        __delay_us(1);
        if (CH375_READ() != USB_INT_DISK_READ) {
            printf("[!] disk_write: Read error\n");
            return -1;
        }
        CH375_A0 = 1;
        CH375_WRITE(RD_USB_DATA);
        __delay_us(1);
        CH375_READ();
        for (int j = i; j < i + 64; j++)
            extern_memory_write(addr + j, CH375_READ());

        CH375_A0 = 1;
        CH375_WRITE(DISK_RD_GO);
        __delay_us(1);
        wait_for_interrupt();
    }

    CH375_CMD(GET_STATUS);
    if (CH375_READ() != USB_INT_SUCCESS)
        return -1;

    return 0;
}

void ch375_gpio_init(void)
{
    ADCON1 = 0b1111;
    TRISA = 0;
    TRISD = 0xFF;
    TRISC2 = 0;
    TRISC3 = 0;
    CH375_WR = 1;
    CH375_RD = 1;
    TRISC0 = 0;
    CH375_CE = 1;
    TRISB0 = 1;
}


void ch375_init()
{
    ch375_gpio_init();
    __delay_ms(40);
    INTEDG0 = 0;
    INT0IF = 0;
    INT0IE = 1;
    CH375_CMD(GET_IC_VAR);
    printf("CH375 IC version: 0x%x\n", CH375_READ());
    set_usb_mode(USB_MODE_SOF);
}
