#include <ch375.h>

struct usb_device_descriptor device_descriptor;
struct usb_config_descriptor config_descriptor;
struct usb_interface_descriptor interface_descriptor;
struct usb_hid_descriptor hid_descriptor;
struct usb_endpoint_descriptor endpoint_descriptor;

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
    __delay_us(100);
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

/**
 * This **strange** declaration is because of the strange behavior of XC8
 * compiler's.
 */
char a[] = "1234567890";
char b[] = "!@#$%^&*()";
char c[] = "\n\x00\b\x00 -=[]\\\x00;'`,./";
char d[] = "\n\x00\x00\x00 _+{}|\x00:\"~<>?";
char *key_arr1[2] = {a, b};
char *key_arr2[2] = {c, d};

/**
 * TODO: Support more keys according to the Chapter 0x7 of the HID Usage Tables
 * https://usb.org/sites/default/files/hut1_4.pdf
 */
static inline char decode_hid_key(uint8_t modifier, uint8_t keycode)
{
    uint8_t shift = !!(modifier & 0x22);
    if (keycode >= 0x04 && keycode <= 0x1D)
        return (shift ? 'A' : 'a') + (keycode - 0x04);

    if (keycode >= 0x1E && keycode <= 0x27)
        return key_arr1[shift][keycode - 0x1E];

    if (keycode >= 0x28 && keycode <= 0x38)
        return key_arr2[shift][keycode - 0x28];
    return 0;
}

#define USB_CONNECTED (1U << 0)
#define USB_TOGGLE (1U << 1)
uint8_t usb_flags = 0;
uint8_t buf[8];
uint8_t last_key;

char kb_input_queue[KB_INPUT_QUEUE_NUM];
kb_info_t kb_info = {0};

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


        CH375_CMD(GET_DESCR);
        CH375_WRITE_DATA(0x02);
        wait_for_interrupt();

        CH375_CMD(GET_STATUS);
        if (CH375_READ() != USB_INT_SUCCESS) {
            printf("[!] Get Config Descriptor Fail\n");
            return;
        }
        printf("[+] Got Config Descriptor\n");

        CH375_CMD(RD_USB_DATA);
        len = CH375_READ();
        if (len < 9)
            return;
        uint8_t *ptr = (uint8_t *) &config_descriptor;
        for (uint8_t i = 0; i < 9; i++)
            *ptr++ = CH375_READ();
        len -= 9;
#ifdef DEBUG
        print_config_descriptor(&config_descriptor);
#endif

        if (len < 9)
            return;
        ptr = (uint8_t *) &interface_descriptor;
        for (uint8_t i = 0; i < 9; i++)
            *ptr++ = CH375_READ();
        len -= 9;
#ifdef DEBUG
        print_interface_descriptor(&interface_descriptor);
#endif

        if (len < 9)
            return;
        ptr = (uint8_t *) &hid_descriptor;
        for (uint8_t i = 0; i < 9; i++)
            *ptr++ = CH375_READ();
        len -= 9;
#ifdef DEBUG
        print_interface_descriptor(&interface_descriptor);
#endif

        if (len < 7)
            return;
        ptr = (uint8_t *) &endpoint_descriptor;
        for (uint8_t i = 0; i < 7; i++)
            *ptr++ = CH375_READ();
        len -= 7;
#ifdef DEBUG
        print_endpoint_descriptor(&endpoint_descriptor);
#endif
        CH375_CMD(SET_CONFIG);
        CH375_WRITE_DATA(config_descriptor.bConfigurationValue);

        wait_for_interrupt();
        CH375_CMD(GET_STATUS);
        if (CH375_READ() != USB_INT_SUCCESS) {
            printf("[!] Set Configuration Value fail\n");
            return;
        }

        CH375_CMD(SET_RETRY);
        CH375_WRITE_DATA(0x25);
        CH375_WRITE_DATA(0x85);

        printf("[+] USB Connected\n");
        usb_flags |= USB_CONNECTED;
        usb_flags &= ~USB_TOGGLE;
        CH375_CMD(SET_ENDP6);
        if (usb_flags & USB_TOGGLE)
            CH375_WRITE_DATA(0xC0);
        else
            CH375_WRITE_DATA(0x80);
        usb_flags ^= USB_TOGGLE;
        uint8_t addr = endpoint_descriptor.bEndpointAddress & 0xF;
        CH375_CMD(ISSUE_TOKEN);
        CH375_WRITE_DATA((uint8_t) (addr << 4) | DEF_USB_PID_IN);
        last_key = 0;
    } else {
        CH375_CMD(GET_STATUS);
        if (CH375_READ() != USB_INT_SUCCESS) {
            CH375_CMD(GET_STATUS);
            uint8_t x = CH375_READ();
            usb_flags &= ~USB_CONNECTED;
            printf("[+] USB Disconnected\n");
            return;
        }

        uint8_t len;
        CH375_CMD(RD_USB_DATA);
        len = CH375_READ();
        if (len <= 8) {
            uint8_t *ptr = buf;
            for (int i = 0; i < len; i++)
                *ptr++ = CH375_READ();

            /** Ignore zero key code and only use the first key code */
            if (buf[2] == 0) {
                last_key = 0;
                goto done;
            }

            if (last_key != buf[2]) {
                if (!kb_queue_full())
                    kb_queue_in(decode_hid_key(buf[0], buf[2]));

                kb_info.int_flag = 1;
            }

            last_key = buf[2];
        }
    done:
        CH375_CMD(SET_ENDP6);
        if (usb_flags & USB_TOGGLE)
            CH375_WRITE_DATA(0xC0);
        else
            CH375_WRITE_DATA(0x80);
        usb_flags ^= USB_TOGGLE;

        uint8_t addr = endpoint_descriptor.bEndpointAddress & 0xF;
        CH375_CMD(ISSUE_TOKEN);
        CH375_WRITE_DATA((uint8_t) (addr << 4) | DEF_USB_PID_IN);
    }
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
    INTCON2bits.INTEDG0 = 0;
    INTCONbits.INT0IF = 0;
    INTCONbits.INT0IE = 1;
    CH375_CMD(GET_IC_VAR);
    printf("CH375 IC version: 0x%x\n", CH375_READ());
    set_usb_mode(USB_MODE_SOF);
}
