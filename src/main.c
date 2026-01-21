#include <ch375.h>
#include <dma.h>
#include <fat32.h>
#include <interrupt.h>
#include <kernel.h>
#include <libc.h>
#include <schedule.h>
#include <usr_libc.h>

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

fat32_t *fs;
addr_t root;

void __attribute__((naked)) task1(void)
{
    while (1) ;
    exit();
}

void __attribute__((naked)) task2(void) 
{
    while (1) {
        usr_uart_put_char('2');
        __delay_ms(1000);
    }
    exit();
}

void __attribute__((naked)) task3(void) {}

spin_lock_t fp_lock = {0};

void __attribute__((naked)) fpstart(void)
{
    usr_uart_put_char('f');
    usr_uart_put_char('\n');
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
    spin_lock(fp_lock);
    fsstart_ans = result;
    spin_unlock(fp_lock);
    exit();
}

void __attribute__((naked)) fpend(void)
{
    uint32_t result;
    int8_t i;

    spin_lock(fp_lock);
    result = fsstart_ans;
    spin_unlock(fp_lock);
    if (result == 0) {
        char *st =
            "e\b\brror: no result available or calculation not finished\r\n$ ";
        for (i = 0; i < 60; i++) {
            usr_uart_put_char(st[i]);
        }
    } else {
        usr_uart_put_char('r');
        usr_uart_put_char(':');

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

            for (i = num_len - 1; i >= 0; i--) {
                usr_uart_put_char(num_str[i]);
            }
        }
        usr_uart_put_char('\r');
        usr_uart_put_char('\n');
        usr_uart_put_char('$');
        usr_uart_put_char(' ');
    }

    spin_lock(fp_lock);
    fsstart_ans = 0;
    spin_unlock(fp_lock);

    exit();
}

uint8_t input_method = 0;  // 0 : uart, 1 : keyboard

void __attribute__((naked)) task4(void)
{
    static char cmd_buffer[32];
    static uint8_t cmd_index = 0;
    uint8_t i;
    char *str;
    while (1) {
        usr_uart_put_char('$');
        usr_uart_put_char(' ');


        cmd_index = 0;
        while (1) {
            char ch;
            if (input_method == 0)
                usr_uart_get_char(ch);
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
                    for (i = 5; i < cmd_index; i++) {
                        usr_uart_put_char(cmd_buffer[i]);
                    }
                    usr_uart_put_char('\r');
                    usr_uart_put_char('\n');
                }

                else if (cmd_index == 4) {
                    str = "Usage: echo <text>\r\n";
                    for (i = 0; i < 20; i++) {
                        usr_uart_put_char(str[i]);
                    }
                }
            }

            else if (cmd_index == 2 && cmd_buffer[0] == 'p' &&
                     cmd_buffer[1] == 's') {
                str = "PID State Stack\r\n------------------\r\n";
                for (i = 0; i < 38; i++) {
                    usr_uart_put_char(str[i]);
                }

                for (i = 0; i < 4; i++) {
                    if (run_task_info & (1 << i)) {
                        usr_uart_put_char('0' + i);
                        usr_uart_put_char(' ');
                        usr_uart_put_char(' ');


                        if (((run_task_info >> 4) & 0x3) == i) {
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
                for (i = 0; i < 14; i++) {
                    usr_uart_put_char(str[i]);
                }

                for (i = 0; i < 4; i++) {
                    if (stack_status.use & (1 << i)) {
                        usr_uart_put_char('1');
                    } else {
                        usr_uart_put_char('0');
                    }
                }
                usr_uart_put_char('\r');
                usr_uart_put_char('\n');

            } else if (cmd_buffer[0] == 'f' && cmd_buffer[1] == 'p' &&
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
                    fsstart_param = ans;
                    GIE = 0;
                    create_process(&fpstart, 0);
                    GIE = 1;
                } else {
                    str = "Error: fpstart requires a number parameter\r\n";
                    for (i = 0; i < 44; i++) {
                        usr_uart_put_char(str[i]);
                    }
                }
            }

            else if (cmd_index == 5 && cmd_buffer[0] == 'f' &&
                     cmd_buffer[1] == 'p' && cmd_buffer[2] == 'e' &&
                     cmd_buffer[3] == 'n' && cmd_buffer[4] == 'd') {
                GIE = 0;
                create_process(&fpend, 0);
                GIE = 1;
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
                        if (run_task[pid].stack_info.stack_size > 0) {
                            GIE = 0;
                            stack_release(pid);
                            GIE = 1;
                        }


                        run_task_info &= (uint8_t) ~(1 << pid);


                        str = "Process killed successfully\r\n";
                        for (i = 0; i < 30; i++) {
                            usr_uart_put_char(str[i]);
                        }
                    } else {
                        str = "Error: Process does not exist\r\n";
                        for (i = 0; i < 32; i++) {
                            usr_uart_put_char(str[i]);
                        }
                    }
                } else if (!has_param) {
                    str = "Error: kill requires a PID parameter\r\n";
                    for (i = 0; i < 38; i++) {
                        usr_uart_put_char(str[i]);
                    }
                } else {
                    char *st = "Error: Invaild PID (must be 0-3)\r\n";
                    for (i = 0; i < 34; i++) {
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
                        str = "keyboard not connect\r\n";
                        for (i = 0; i < sizeof(str); i++)
                            usr_uart_put_char(str[i]);
                    } else
                        input_method = 1;
                } else
                    input_method = 0;
            } else if (cmd_index == 2 && cmd_buffer[0] == 'l' &&
                       cmd_buffer[1] == 's') {
                GIE = 0;
                ls_dir((addr_t) root);
                GIE = 1;
            } else if (cmd_index >= 3 && cmd_buffer[0] == 'c' &&
                       cmd_buffer[1] == 'a' && cmd_buffer[2] == 't') {
                uint8_t idx = 3;

                while (idx < cmd_index && cmd_buffer[idx] == ' ')
                    idx++;
                if (idx == cmd_index) {
                    str = "Usage: cat [file]\r\n";
                    for (uint8_t i = 0; i < 19; i++) {
                        usr_uart_put_char(str[i]);
                    }
                    continue;
                }
                GIE = 0;
                addr_t target_addr =
                    find_file(root, &cmd_buffer[idx], cmd_index - idx);
                if (target_addr != EXTERN_NULL) {
                    extern_memory_read((uint16_t) (target_addr >> 6),
                                       (char *) picos_cache);
                    file_t *target = (file_t *) picos_cache;
                    if (target->file_size == 0)
                        continue;
                    addr_t target_buf;
                    extern_alloc(8, target_buf);
                    read_file(fs, target, target_buf, 512);
                    for (uint16_t i = 0; i < 512; i += 64) {
                        extern_memory_read((uint16_t) ((target_buf + i) >> 6),
                                           (char *) picos_cache);
                        for (uint16_t j = 0; j < 64; j++) {
                            putchar(picos_cache[j]);
                            if (i + j == target->file_size)
                                goto end_cat;
                        }
                    }
                end_cat:
                }
                GIE = 1;
            } else if (cmd_index == 5 && cmd_buffer[0] == 'm' &&
                       cmd_buffer[1] == 'u' && cmd_buffer[2] == 'l' && cmd_buffer[3] == 't' && cmd_buffer[4] == 'i') {
                GIE = 0;
                create_process(task2, 0);
                GIE = 1;
            } else {
                usr_uart_put_char('?');
                usr_uart_put_char(' ');
                for (i = 0; i < cmd_index; i++) {
                    usr_uart_put_char(cmd_buffer[i]);
                }
                str = "\r\nTry: echo, ps, fpstart, fpend, kill, switch_in\r\n";
                for (i = 0; i < 51; i++) {
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

    printf("boot\n");

    dma_init();
    extern_memory_init();
    ch375_init();
    __delay_ms(3000);

    alloc_init();

    fs = create_fat32();
    root = load_dir(fs, fs->root_clus);

    INTCONbits.GIE = 1;

    ADCON1 = 0xF;
    timer0_init();

    init_scheduler();

    create_process(&task4, 2);
    create_process(&task1, 0);

    
    start_schedule();

    PANIC("hello\n");
}
