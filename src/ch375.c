#include <ch375.h>

#pragma interrupt_level 1
#pragma interrupt_level 2
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

#pragma interrupt_level 1
#pragma interrupt_level 2
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
 * TODO: Support more keys according to the Chapter 0x7 of the HID Usage Tables
 * https://usb.org/sites/default/files/hut1_4.pdf
 *
 * Implement decode without lookup arrays: return character per keycode
 * using explicit conditionals so the compiler doesn't rely on pointer data.
 */
static char decode_hid_key(uint8_t modifier, uint8_t keycode)
{
    uint8_t shift = !!(modifier & 0x22);

    if (keycode >= 0x04 && keycode <= 0x1D)
        return (shift ? 'A' : 'a') + (keycode - 0x04);

    if (keycode == 0x2C)
        return ' ';

    if (keycode >= 0x1E && keycode <= 0x27) {
        if (shift) {
            switch (keycode) {
            case 0x1E:
                return '!';
            case 0x1F:
                return '@';
            case 0x20:
                return '#';
            case 0x21:
                return '$';
            case 0x22:
                return '%';
            case 0x23:
                return '^';
            case 0x24:
                return '&';
            case 0x25:
                return '*';
            case 0x26:
                return '(';
            case 0x27:
                return ')';
            }
        } else {
            switch (keycode) {
            case 0x1E:
                return '1';
            case 0x1F:
                return '2';
            case 0x20:
                return '3';
            case 0x21:
                return '4';
            case 0x22:
                return '5';
            case 0x23:
                return '6';
            case 0x24:
                return '7';
            case 0x25:
                return '8';
            case 0x26:
                return '9';
            case 0x27:
                return '0';
            }
        }
    }

    if (keycode >= 0x28 && keycode <= 0x38) {
        if (shift) {
            switch (keycode) {
            case 0x28:
                return '\n';
            case 0x2C:
                return ' ';
            case 0x2D:
                return '_';
            case 0x2E:
                return '+';
            case 0x2F:
                return '{';
            case 0x30:
                return '}';
            case 0x31:
                return '|';
            case 0x33:
                return ':';
            case 0x34:
                return '"';
            case 0x35:
                return '~';
            case 0x36:
                return '<';
            case 0x37:
                return '>';
            case 0x38:
                return '?';
            }
        } else {
            switch (keycode) {
            case 0x28:
                return '\n';
            case 0x2A:
                return '\b';
            case 0x2C:
                return ' ';
            case 0x2D:
                return '-';
            case 0x2E:
                return '=';
            case 0x2F:
                return '[';
            case 0x30:
                return ']';
            case 0x31:
                return '\\';
            case 0x33:
                return ';';
            case 0x34:
                return '\'';
            case 0x35:
                return '`';
            case 0x36:
                return ',';
            case 0x37:
                return '.';
            case 0x38:
                return '/';
            }
        }
    }

    return 0;
}

uint8_t usb_flags = 0;
uint8_t buf[9];
uint8_t last_key;

char kb_input_queue[KB_INPUT_QUEUE_NUM];
kb_info_t kb_info = {0};

uint8_t bEndpointAddress;

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
        if (len == 18)
            for (uint8_t i = 0; i < len; i++)
                CH375_READ();

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

        for (uint8_t i = 0; i < 9; i++)
            buf[i] = CH375_READ();
        len -= 9;

        uint8_t bConfigurationValue =
            ((struct usb_config_descriptor *) buf)->bConfigurationValue;


        if (len < 9)
            return;

        for (uint8_t i = 0; i < 9; i++)
            CH375_READ();
        len -= 9;

        if (len < 9)
            return;

        for (uint8_t i = 0; i < 9; i++)
            CH375_READ();
        len -= 9;


        if (len < 7)
            return;

        for (uint8_t i = 0; i < 7; i++)
            buf[i] = CH375_READ();
        len -= 7;
        bEndpointAddress =
            ((struct usb_endpoint_descriptor *) buf)->bEndpointAddress & 0xF;


        CH375_CMD(SET_CONFIG);
        CH375_WRITE_DATA(bConfigurationValue);

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

        CH375_CMD(ISSUE_TOKEN);
        CH375_WRITE_DATA((uint8_t) (bEndpointAddress << 4) | DEF_USB_PID_IN);
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
                if (!kb_queue_full()) {
                    // printf("key: %x, %x, %c\n", buf[0], buf[2],
                    // decode_hid_key(buf[0], buf[2]));
                    kb_queue_in(decode_hid_key(buf[0], buf[2]));
                }

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

        CH375_CMD(ISSUE_TOKEN);
        CH375_WRITE_DATA((uint8_t) (bEndpointAddress << 4) | DEF_USB_PID_IN);
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
