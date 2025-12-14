#include <libc.h>

void putchar(char c)
{
    if (c == '\n')
        uart_putchar('\r');
    if (c == '\b') {
        uart_putchar('\b');
        uart_putchar(' ');
    }
    uart_putchar(c);
}


static void print_uint(unsigned int x)
{
    if (x == 0) {
        putchar('0');
        return;
    }
    char tmp[10];
    int i = 0;
    tmp[0] = '0';
    while (x) {
        tmp[i++] = x % 10 + '0';
        x /= 10;
    }
    i--;
    while (i >= 0)
        putchar(tmp[i--]);
}

void printf(const char *fmt, ...)
{
    va_list vargs;
    va_start(vargs, fmt);
    for (; *fmt; fmt++) {
        if (*fmt != '%') {
            putchar(*fmt);
            continue;
        }
        fmt++;
        switch (*fmt) {
        case '\0':
            goto DONE;
        case 's': {
            const char *s = va_arg(vargs, const char *);
            while (*s)
                putchar(*s++);
            break;
        }
        case 'c':
            putchar(*fmt);
            break;
        case 'd': {
            int x = va_arg(vargs, int);
            if (x < 0) {
                putchar('-');
                x = -x;
            }
            print_uint((unsigned int) x);
            break;
        }
        case 'u': {
            unsigned int y = va_arg(vargs, unsigned int);
            print_uint(y);
            break;
        }
        case 'x': {
            unsigned int z = va_arg(vargs, unsigned int);
            for (int i = 7; i >= 0; i--) {
                putchar("0123456789abcdef"[(z >> (i << 2)) & 0xf]);
            }
            break;
        }
        case 'l': {
            fmt++;
            unsigned long z = va_arg(vargs, unsigned long);
            for (int i = 15; i >= 0; i--) {
                putchar("0123456789abcdef"[(z >> (i << 2)) & 0xf]);
            }
            break;
        }
        }
    }
DONE:
    va_end(vargs);
}
