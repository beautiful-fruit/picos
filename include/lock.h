#pragma once

typedef union {
    struct {
        unsigned nop : 6;
        unsigned ret : 1;
        unsigned lock : 1;
    };
} spin_lock_t;

#define spin_lock(_lock)            \
    do {                            \
        while (1) {                 \
            INTCONbits.GIE = 0;     \
            if (_lock.lock == 0) {  \
                _lock.lock = 1;     \
                INTCONbits.GIE = 1; \
                break;              \
            }                       \
            INTCONbits.GIE = 1;     \
        }                           \
    } while (0)

#define try_spin_lock(_lock, result) \
    do {                             \
        INTCONbits.GIE = 0;          \
        if (_lock.lock == 0) {       \
            _lock.lock = 1;          \
            result = 1;              \
        } else                       \
            result = 0;              \
        INTCONbits.GIE = 1;          \
    } while (0)

#define spin_unlock(_lock) (_lock.lock = 0)
