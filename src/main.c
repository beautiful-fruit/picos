#include <ch375.h>
#include <dma.h>
#include <interrupt.h>
#include <kernel.h>
#include <libc.h>
#include <schedule.h>
#include <tests.h>
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

void __attribute__((naked)) task1(void)
{
    while (1)
        ;
    exit();
}

void __attribute__((naked)) task2(void)
{
    lock();
    usr_uart_put_char('0' + ((run_task_info >> 4) & 0x3));
    usr_uart_put_char(':');
    unlock();
    lock();
    usr_uart_put_char('2');
    usr_uart_put_char('\r');
    usr_uart_put_char('\n');
    unlock();
    exit();
}

void __attribute__((naked)) task3(void)
{
    while (1) {
        int_wait_queue_push(0);
        lock();
        usr_uart_put_char('3');
        unlock();
    }
    exit();
}

void __attribute__((naked)) task4(void)
{
    static char cmd_buffer[32];
    static uint8_t cmd_index = 0;

    while (1) {
        lock();


        usr_uart_put_char('$');
        usr_uart_put_char(' ');


        cmd_index = 0;
        while (1) {
            char ch;
            usr_uart_get_char(ch);


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
                    /*
                    usr_uart_put_char('U');
                    usr_uart_put_char('s');
                    usr_uart_put_char('a');
                    usr_uart_put_char('g');
                    usr_uart_put_char('e');
                    usr_uart_put_char(':');
                    usr_uart_put_char(' ');
                    usr_uart_put_char('e');
                    usr_uart_put_char('c');
                    usr_uart_put_char('h');
                    usr_uart_put_char('o');
                    usr_uart_put_char(' ');
                    usr_uart_put_char('<');
                    usr_uart_put_char('t');
                    usr_uart_put_char('e');
                    usr_uart_put_char('x');
                    usr_uart_put_char('t');
                    usr_uart_put_char('>');
                    usr_uart_put_char('\r');
                    usr_uart_put_char('\n');
                    */
                    char *str = "Usage: echo <text>\r\n";
                    for (uint8_t i = 0; i < 20; i++) {
                        usr_uart_put_char(str[i]);
                    }
                }
            }

            else if (cmd_index == 2 && cmd_buffer[0] == 'p' &&
                     cmd_buffer[1] == 's') {
                /*
                usr_uart_put_char('P');
                usr_uart_put_char('I');
                usr_uart_put_char('D');
                usr_uart_put_char(' ');
                usr_uart_put_char('S');
                usr_uart_put_char('t');
                usr_uart_put_char('a');
                usr_uart_put_char('t');
                usr_uart_put_char('e');
                usr_uart_put_char(' ');
                usr_uart_put_char('S');
                usr_uart_put_char('t');
                usr_uart_put_char('a');
                usr_uart_put_char('c');
                usr_uart_put_char('k');
                usr_uart_put_char('\r');
                usr_uart_put_char('\n');
                usr_uart_put_char('-');
                usr_uart_put_char('-');
                usr_uart_put_char('-');
                usr_uart_put_char('-');
                usr_uart_put_char('-');
                usr_uart_put_char('-');
                usr_uart_put_char('-');
                usr_uart_put_char('-');
                usr_uart_put_char('-');
                usr_uart_put_char('-');
                usr_uart_put_char('-');
                usr_uart_put_char('-');
                usr_uart_put_char('-');
                usr_uart_put_char('\r');
                usr_uart_put_char('\n');
                */
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

                /*
                usr_uart_put_char('\r');
                usr_uart_put_char('\n');
                usr_uart_put_char('S');
                usr_uart_put_char('t');
                usr_uart_put_char('a');
                usr_uart_put_char('c');
                usr_uart_put_char('k');
                usr_uart_put_char(' ');
                usr_uart_put_char('u');
                usr_uart_put_char('s');
                usr_uart_put_char('e');
                usr_uart_put_char(':');
                usr_uart_put_char(' ');
                */
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

            else if (cmd_index == 5 && cmd_buffer[0] == 'f' &&
                     cmd_buffer[1] == 's' && cmd_buffer[2] == 'e' &&
                     cmd_buffer[3] == 'n' && cmd_buffer[4] == 'd') {
            }

            else {
                usr_uart_put_char('?');
                usr_uart_put_char(' ');
                for (uint8_t i = 0; i < cmd_index; i++) {
                    usr_uart_put_char(cmd_buffer[i]);
                }
                /*
                usr_uart_put_char('\r');
                usr_uart_put_char('\n');
                usr_uart_put_char('T');
                usr_uart_put_char('r');
                usr_uart_put_char('y');
                usr_uart_put_char(':');
                usr_uart_put_char(' ');
                usr_uart_put_char('e');
                usr_uart_put_char('c');
                usr_uart_put_char('h');
                usr_uart_put_char('o');
                usr_uart_put_char(',');
                usr_uart_put_char(' ');
                usr_uart_put_char('p');
                usr_uart_put_char('s');
                usr_uart_put_char('\r');
                usr_uart_put_char('\n');
                */
                char *str = "\r\nTry: echo, ps\r\n";
                for (uint8_t i = 0; i < 18; i++) {
                    usr_uart_put_char(str[i]);
                }
            }
        }

        unlock();
    }
    exit();
}


void main(void)
{
    // Make the return address stack empty
    GIE = 0;
    STKPTR &= 0xE0;
    uart_init();
    // dma_init();
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