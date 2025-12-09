#include <usr_libc.h>

spin_lock_t uart_put_lock = {0};
spin_lock_t uart_get_lock = {0};
