#pragma once
#include <libc.h>
#include <usb.h>

#define GET_IC_VAR 0x01
#define SET_USB_SPEED 0x04
#define SET_RETRY 0x0B
#define SET_USB_ADDR 0x13
#define SET_USB_MODE 0x15
#define SET_ENDP6 0x1C
#define GET_STATUS 0x22
#define RD_USB_DATA 0x28
#define SET_ADDRESS 0x45
#define ISSUE_TOKEN 0x4F
#define GET_DESCR 0x46
#define SET_CONFIG 0x49


#define USB_INT_SUCCESS 0x14
#define USB_INT_CONNECT 0x15
#define USB_INT_DISCONNECT 0x16


#define USB_MODE_NOSOF 0x05
#define USB_MODE_SOF 0x06
#define USB_MODE_RESET 0x07

#define USB_SPEED_FULL_SPEED 0x00
#define USB_SPEED_LOW_SPEED 0x02

#define DEF_USB_PID_IN 0x09

inline uint8_t CH375_READ();
inline void CH375_WRITE(uint8_t data);

#define CH375_A0 LATA4
#define CH375_WR LATA0
#define CH375_RD LATA1
#define CH375_CE LATC0


#define CH375_CMD(x)     \
    do {                 \
        CH375_A0 = 1;    \
        CH375_WRITE(x);  \
        __delay_us(100); \
    } while (0)

#define CH375_WRITE_DATA(x) \
    do {                    \
        CH375_A0 = 0;       \
        CH375_WRITE(x);     \
        __delay_us(100);    \
    } while (0)


#define set_usb_mode(mode)       \
    do {                         \
        CH375_CMD(SET_USB_MODE); \
        CH375_WRITE_DATA(mode);  \
        __delay_us(100);         \
    } while (0)



void usb_handler();

void ch375_init();
