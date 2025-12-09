#include <atomic.h>

typedef union {
    struct {
        unsigned nop : 6;
        unsigned ret : 1;
        unsigned lock : 1;
    };
} spin_lock_t;

#define spin_lock(_lock)                          \
    do {                                          \
        while (1) {                               \
            cmpxchg(_lock.lock, 0, 1, _lock.ret); \
            if (_lock.ret)                        \
                break;                            \
        }                                         \
        _lock.ret = 0;                            \
    } while (0)

#define spin_unlock(_lock) (_lock.lock = 0)
