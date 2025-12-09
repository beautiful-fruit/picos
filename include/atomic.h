#define cmpxchg(obj, expected, desired, ret) \
    do {                                     \
        INTCONbits.GIE = 0;                  \
        if (obj == expected) {               \
            obj = desired;                   \
            ret = 1;                         \
        } else                               \
            ret = 0;                         \
        INTCONbits.GIE = 1;                  \
    } while (0)
