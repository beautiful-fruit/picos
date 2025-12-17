#include <ch375.h>
#include <dma.h>
#include <interrupt.h>
#include <kernel.h>
#include <libc.h>
#include <schedule.h>
#include <fat32.h>
#include <usr_libc.h>
#include <tests.h>

#define test_add(arg1, arg2, output) \
    do {                             \
        *(current->sp) = arg1;       \
        *(current->sp + 1) = arg2;   \
        current->sp += 6;            \
        asm("CALL _test_add_impl");  \
        current->sp -= 6;            \
        output = *(current->sp + 2); \
    } while (0)

void __attribute__((naked)) test_add_impl(void)
{
#define ret (*(current->sp - 4))
#define arg1 (*(current->sp - 6))
#define arg2 (*(current->sp - 5))
    enter_user_func();
    ret = arg1 + arg2;
#undef ret
#undef arg1
#undef arg2
    return_user_func();
}

#define MOD 1000000009UL

uint32_t fsstart_param = 0;
uint32_t fsstart_ans = 0;


void __attribute__((naked)) task1(void)
{
    while (1)
        ;
    exit();
}

void __attribute__((naked)) task2(void) {}

void __attribute__((naked)) task3(void) {}

void __attribute__((naked)) fpstart(void)
{
    uint32_t n = fsstart_param;
    uint32_t result = 0;

    if (n == 0) {
        result = 0;
    } else if (n == 1) {
        result = 1;
    } else {
        uint32_t a = 0;
        uint32_t b = 1;
        uint32_t c = 0;


        for (uint32_t i = 2; i <= n; i++) {
            lock();


            uint32_t block_end = (i + 49 < n) ? i + 49 : n;
            for (uint32_t j = i; j <= block_end; j++) {
                c = (a + b) % MOD;
                a = b;
                b = c;
            }

            unlock();

            i = block_end;
        }
        result = c;
    }

    spin_lock_t ll;
    spin_lock(ll);
    fsstart_ans = result;
    spin_unlock(ll);

    exit();
}

void __attribute__((naked)) fpend(void)
{
    uint32_t result;



    spin_lock_t ll;
    spin_lock(ll);
    result = fsstart_ans;
    spin_unlock(ll);

    if (result == 0) {
        char *st =
            "e\b\brror: no result available or calculation not finished\r\n$ ";
        for (uint8_t i = 0; i < 60; i++) {
            usr_uart_put_char(st[i]);
        }
    } else {
        char *st2 = "r\b\besult: ";
        for (uint8_t i = 0; i < 11; i++) {
            usr_uart_put_char(st2[i]);
        }

        static char num_str[16];
        uint8_t num_len = 0;

        if (result == 0) {
            usr_uart_put_char('0');
        } else {
            uint32_t temp = result;
            while (temp > 0) {
                num_str[num_len++] = (temp % 10) + '0';
                temp /= 10;
            }

            for (int8_t i = num_len - 1; i >= 0; i--) {
                usr_uart_put_char(num_str[i]);
            }
        }
        usr_uart_put_char('\r');
        usr_uart_put_char('\n');
        usr_uart_put_char('$');
        usr_uart_put_char(' ');
    }


    spin_lock_t ll2;
    spin_lock(ll2);
    fsstart_ans = 0;
    spin_unlock(ll2);

    exit();
}

uint8_t input_method = 0;  // 0 : uart, 1 : keyboard

void __attribute__((naked)) task4(void)
{
    static char cmd_buffer[32];
    static uint8_t cmd_index = 0;

    while (1) {
        usr_uart_put_char('$');
        usr_uart_put_char(' ');


        cmd_index = 0;
        while (1) {
            char ch;
            if (input_method == 0)
                ch = usr_uart_get_char();
            else
                usr_kb_get_char(ch);


            if (ch == '\r' || ch == '\n') {
                usr_uart_put_char('\r');
                usr_uart_put_char('\n');
                cmd_buffer[cmd_index] = '\0';
                break;
            }

            else if (ch == '\b' || ch == 0x7F) {
                if (cmd_index > 0) {
                    cmd_index--;
                    usr_uart_put_char('\b');
                    usr_uart_put_char(' ');
                    usr_uart_put_char('\b');
                }
            }

            else if (ch >= 32 && ch <= 126 && cmd_index < 31) {
                usr_uart_put_char(ch);
                cmd_buffer[cmd_index++] = ch;
            }
        }


        if (cmd_index > 0) {
            if (cmd_index >= 4 && cmd_buffer[0] == 'e' &&
                cmd_buffer[1] == 'c' && cmd_buffer[2] == 'h' &&
                cmd_buffer[3] == 'o') {
                if (cmd_index > 4 && cmd_buffer[4] == ' ') {
                    for (uint8_t i = 5; i < cmd_index; i++) {
                        usr_uart_put_char(cmd_buffer[i]);
                    }
                    usr_uart_put_char('\r');
                    usr_uart_put_char('\n');
                }

                else if (cmd_index == 4) {
                    char *str = "Usage: echo <text>\r\n";
                    for (uint8_t i = 0; i < 20; i++) {
                        usr_uart_put_char(str[i]);
                    }
                }
            }

            else if (cmd_index == 2 && cmd_buffer[0] == 'p' &&
                     cmd_buffer[1] == 's') {
                char *str = "PID State Stack\r\n------------------\r\n";
                for (uint8_t i = 0; i < 38; i++) {
                    usr_uart_put_char(str[i]);
                }

                for (uint8_t i = 0; i < 4; i++) {
                    if (run_task_info & (1 << i)) {
                        usr_uart_put_char('0' + i);
                        usr_uart_put_char(' ');
                        usr_uart_put_char(' ');


                        if ((run_task_info >> 4) & 0x3 == i) {
                            usr_uart_put_char('R');  // Running
                        } else if (wait_task_info & (1 << i)) {
                            usr_uart_put_char('W');  // Waiting
                        } else {
                            usr_uart_put_char('S');  // Sleeping/Ready
                        }


                        if (run_task_info & RUN_TASK_EXIT) {
                            usr_uart_put_char('X');
                        } else {
                            usr_uart_put_char(' ');
                        }

                        usr_uart_put_char(' ');
                        usr_uart_put_char(' ');


                        if (run_task[i].stack_info.stack_size > 0) {
                            usr_uart_put_char(
                                '0' + run_task[i].stack_info.stack_start);
                            usr_uart_put_char('-');
                            usr_uart_put_char(
                                '0' + (run_task[i].stack_info.stack_start +
                                       run_task[i].stack_info.stack_size - 1));
                        } else {
                            usr_uart_put_char('N');
                            usr_uart_put_char('/');
                            usr_uart_put_char('A');
                        }

                        usr_uart_put_char('\r');
                        usr_uart_put_char('\n');
                    }
                }



                str = "\r\nStack use: ";
                for (uint8_t i = 0; i < 14; i++) {
                    usr_uart_put_char(str[i]);
                }

                for (uint8_t i = 0; i < 4; i++) {
                    if (stack_status.use & (1 << i)) {
                        usr_uart_put_char('1');
                    } else {
                        usr_uart_put_char('0');
                    }
                }
                usr_uart_put_char('\r');
                usr_uart_put_char('\n');

            } else if (cmd_index == 7 && cmd_buffer[0] == 'f' &&
                       cmd_buffer[1] == 's' && cmd_buffer[2] == 's' &&
                       cmd_buffer[3] == 't' && cmd_buffer[4] == 'a' &&
                       cmd_buffer[5] == 'r' && cmd_buffer[6] == 't') {
            }

            else if (cmd_buffer[0] == 'f' && cmd_buffer[1] == 'p' &&
                     cmd_buffer[2] == 's' && cmd_buffer[3] == 't' &&
                     cmd_buffer[4] == 'a' && cmd_buffer[5] == 'r' &&
                     cmd_buffer[6] == 't') {
                uint32_t ans = 0;
                uint8_t has_param = 0;


                if (cmd_index >= 8 && cmd_buffer[7] == ' ') {
                    has_param = 1;

                    uint8_t param_index = 8;
                    while (param_index < cmd_index) {
                        if (cmd_buffer[param_index] >= '0' &&
                            cmd_buffer[param_index] <= '9') {
                            ans = ans * 10 + (cmd_buffer[param_index] - '0');
                        } else {
                            has_param = 0;
                            break;
                        }
                        param_index++;
                    }
                }

                if (has_param) {
                    spin_lock_t ll;
                    spin_lock(ll);
                    fsstart_param = ans;
                    create_process(&fpstart, 0);
                    spin_unlock(ll);
                } else {
                    char *str =
                        "Error: fpstart requires a number parameter\r\n";
                    for (uint8_t i = 0; i < 44; i++) {
                        usr_uart_put_char(str[i]);
                    }
                }
            }

            else if (cmd_index == 5 && cmd_buffer[0] == 'f' &&
                     cmd_buffer[1] == 'p' && cmd_buffer[2] == 'e' &&
                     cmd_buffer[3] == 'n' && cmd_buffer[4] == 'd') {
                spin_lock_t ll;
                spin_lock(ll);
                create_process(&fpend, 0);
                spin_unlock(ll);

            }

            else if (cmd_buffer[0] == 'k' && cmd_buffer[1] == 'i' &&
                     cmd_buffer[2] == 'l' && cmd_buffer[3] == 'l') {
                uint8_t pid = 0;
                uint8_t has_param = 0;

                if (cmd_index >= 5 && cmd_buffer[4] == ' ') {
                    has_param = 1;

                    uint8_t param_index = 5;
                    while (param_index < cmd_index) {
                        if (cmd_buffer[param_index] >= '0' &&
                            cmd_buffer[param_index] <= '9') {
                            pid = pid * 10 + (cmd_buffer[param_index] - '0');
                        } else {
                            has_param = 0;
                            break;
                        }
                        param_index++;
                    }
                }

                if (has_param && pid < RUN_TASK_SIZE) {
                    if (run_task_info & (1 << pid)) {
                        spin_lock_t ll;
                        spin_lock(ll);



                        if (run_task[pid].stack_info.stack_size > 0) {
                            stack_release(pid);
                        }


                        run_task_info &= (uint8_t) ~(1 << pid);

                        spin_unlock(ll);

                        char *str = "Process killed successfully\r\n";
                        for (uint8_t i = 0; i < 30; i++) {
                            usr_uart_put_char(str[i]);
                        }
                    } else {
                        char *str = "Error: Process does not exist\r\n";
                        for (uint8_t i = 0; i < 32; i++) {
                            usr_uart_put_char(str[i]);
                        }
                    }
                } else if (!has_param) {
                    char *str = "Error: kill requires a PID parameter\r\n";
                    for (uint8_t i = 0; i < 38; i++) {
                        usr_uart_put_char(str[i]);
                    }
                } else {
                    char str[40];

                    char *st = "Error: Invaild PID (must be 0-3)\r\n";
                    for (uint8_t i = 0; i < 34; i++) {
                        usr_uart_put_char(st[i]);
                    }
                }
            } else if (cmd_buffer[0] == 's' && cmd_buffer[1] == 'w' &&
                       cmd_buffer[2] == 'i' && cmd_buffer[3] == 't' &&
                       cmd_buffer[4] == 'c' && cmd_buffer[5] == 'h' &&
                       cmd_buffer[6] == '_' && cmd_buffer[7] == 'i' &&
                       cmd_buffer[8] == 'n') {
                if (!input_method) {
                    if (!(usb_flags & USB_CONNECTED)) {
                        char str[] = "keyboard not connect\r\n";
                        for (uint8_t i = 0; i < sizeof(str); i++)
                            usr_uart_put_char(str[i]);
                    } else
                        input_method = 1;
                } else
                    input_method = 0;
            } else {
                usr_uart_put_char('?');
                usr_uart_put_char(' ');
                for (uint8_t i = 0; i < cmd_index; i++) {
                    usr_uart_put_char(cmd_buffer[i]);
                }


                char *str =
                    "\r\nTry: echo, ps, fpstart, fpend, kill, switch_in\r\n";
                for (uint8_t i = 0; i < 51; i++) {
                    usr_uart_put_char(str[i]);
                }
            }
        }
    }
    exit();
}


void main(void)
{
    // Make the return address stack empty
    GIE = 0;
    STKPTR &= 0xE0;
    uart_init();
    dma_init();
    extern_memory_init();
    ch375_init();
    __delay_ms(3000);
    INTCONbits.GIE = 1;

    ADCON1 = 0xF;
    timer0_init();

    init_scheduler();

    create_process(&task4, 2);
    create_process(&task1, 0);


    start_schedule();

    PANIC("hello\n");
}
