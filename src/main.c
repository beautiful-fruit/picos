#include <libc.h>
#include <stdarg.h>

void main(void)
{
    uart_init();
    TRISBbits.TRISB0 = 0;
    LATBbits.LATB0 = 0;
    PANIC("hello\n");
}
