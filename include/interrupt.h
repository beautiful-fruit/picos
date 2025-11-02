#include <libc.h>

#define set_timer_delay(t)                 \
    do {                                   \
        T0CONbits.TMR0ON = 0;              \
        TMR0H = ((65536 - t) >> 8) & 0xFF; \
        TMR0L = (65536 - t) & 0xFF;        \
        T0CONbits.TMR0ON = 1;              \
    } while (0)

#define timer_disable() INTCONbits.TMR0IE = 0
#define timer_enable() INTCONbits.TMR0IE = 1

#define timer_pause() T0CONbits.TMR0ON = 0;
#define timer_continue() T0CONbits.TMR0ON = 1;

inline void timer0_init(void);

void __attribute__((naked)) isr(void);

#define lock() timer_disable()

#define unlock() timer_enable()
