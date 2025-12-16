#include <ch375.h>
#include <dma.h>
#include <interrupt.h>
#include <kernel.h>
#include <libc.h>
#include <schedule.h>
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

void __attribute__((naked)) task1(void)
{
    lock();
    uart_putchar('0' + ((run_task_info >> 4) & 0x3));
    uart_putchar(':');
    unlock();
    lock();
    uart_putchar('x');
    uart_putchar('\r');
    uart_putchar('\n');
    unlock();
    exit();
}

void __attribute__((naked)) task2(void)
{
    lock();
    uart_putchar('0' + ((run_task_info >> 4) & 0x3));
    uart_putchar(':');
    unlock();
    lock();
    uart_putchar('2');
    uart_putchar('\r');
    uart_putchar('\n');
    unlock();
    exit();
}

void __attribute__((naked)) task3(void)
{
    while (1) {
        int_wait_queue_push(0);
        lock();
        uart_putchar('3');
        unlock();
    }
    exit();
}

void __attribute__((naked)) task4(void)
{
    static char cmd_buffer[32];
    static uint8_t cmd_index = 0;
    
    while(1) {
        lock();
        
        
        uart_putchar('$');
        uart_putchar(' ');
        
        
        cmd_index = 0;
        while(1) {
            char ch = uart_getchar();
            
            
            if (ch == '\r' || ch == '\n') {
                uart_putchar('\r');
                uart_putchar('\n');
                cmd_buffer[cmd_index] = '\0'; 
                break;
            }
            
            else if (ch == '\b' || ch == 0x7F) {
                if (cmd_index > 0) {
                    cmd_index--;
                    uart_putchar('\b');
                    uart_putchar(' ');
                    uart_putchar('\b');
                }
            }
            
            else if (ch >= 32 && ch <= 126 && cmd_index < 31) {
                uart_putchar(ch);  
                cmd_buffer[cmd_index++] = ch;
            }
        }
        
        
        if (cmd_index > 0) {
            
            if (cmd_index >= 4 && 
                cmd_buffer[0] == 'e' && 
                cmd_buffer[1] == 'c' && 
                cmd_buffer[2] == 'h' && 
                cmd_buffer[3] == 'o') {
                
                
                if (cmd_index > 4 && cmd_buffer[4] == ' ') {
                    
                    for (uint8_t i = 5; i < cmd_index; i++) {
                        uart_putchar(cmd_buffer[i]);
                    }
                    uart_putchar('\r');
                    uart_putchar('\n');
                }
                
                else if (cmd_index == 4) {
                    
                    uart_putchar('U');
                    uart_putchar('s');
                    uart_putchar('a');
                    uart_putchar('g');
                    uart_putchar('e');
                    uart_putchar(':');
                    uart_putchar(' ');
                    uart_putchar('e');
                    uart_putchar('c');
                    uart_putchar('h');
                    uart_putchar('o');
                    uart_putchar(' ');
                    uart_putchar('<');
                    uart_putchar('t');
                    uart_putchar('e');
                    uart_putchar('x');
                    uart_putchar('t');
                    uart_putchar('>');
                    uart_putchar('\r');
                    uart_putchar('\n');
                }
            }
            
            else if (cmd_index == 2 && 
                     cmd_buffer[0] == 'p' && 
                     cmd_buffer[1] == 's') {
                
                uart_putchar('P');
                uart_putchar('I');
                uart_putchar('D');
                uart_putchar(' ');
                uart_putchar('S');
                uart_putchar('t');
                uart_putchar('a');
                uart_putchar('t');
                uart_putchar('e');
                uart_putchar(' ');
                uart_putchar('S');
                uart_putchar('t');
                uart_putchar('a');
                uart_putchar('c');
                uart_putchar('k');
                uart_putchar('\r');
                uart_putchar('\n');
                uart_putchar('-');
                uart_putchar('-');
                uart_putchar('-');
                uart_putchar('-');
                uart_putchar('-');
                uart_putchar('-');
                uart_putchar('-');
                uart_putchar('-');
                uart_putchar('-');
                uart_putchar('-');
                uart_putchar('-');
                uart_putchar('-');
                uart_putchar('-');
                uart_putchar('\r');
                uart_putchar('\n');
                
                
                for (uint8_t i = 0; i < 4; i++) {
                    if (run_task_info & (1 << i)) {
                        
                        uart_putchar('0' + i);
                        uart_putchar(' ');
                        uart_putchar(' ');
                        
                        
                        if ((run_task_info >> 4) & 0x3 == i) {
                            uart_putchar('R');  // Running
                        } else if (wait_task_info & (1 << i)) {
                            uart_putchar('W');  // Waiting
                        } else {
                            uart_putchar('S');  // Sleeping/Ready
                        }
                        
                        
                        if (run_task_info & RUN_TASK_EXIT) {
                            uart_putchar('X');  
                        } else {
                            uart_putchar(' ');
                        }
                        
                        uart_putchar(' ');
                        uart_putchar(' ');
                        
                        
                        if (run_task[i].stack_info.stack_size > 0) {
                            uart_putchar('0' + run_task[i].stack_info.stack_start);
                            uart_putchar('-');
                            uart_putchar('0' + (run_task[i].stack_info.stack_start + 
                                               run_task[i].stack_info.stack_size - 1));
                        } else {
                            uart_putchar('N');
                            uart_putchar('/');
                            uart_putchar('A');
                        }
                        
                        uart_putchar('\r');
                        uart_putchar('\n');
                    }
                }
                
                
                uart_putchar('\r');
                uart_putchar('\n');
                uart_putchar('S');
                uart_putchar('t');
                uart_putchar('a');
                uart_putchar('c');
                uart_putchar('k');
                uart_putchar(' ');
                uart_putchar('u');
                uart_putchar('s');
                uart_putchar('e');
                uart_putchar(':');
                uart_putchar(' ');
                
                for (uint8_t i = 0; i < 4; i++) {
                    if (stack_status.use & (1 << i)) {
                        uart_putchar('1');
                    } else {
                        uart_putchar('0');
                    }
                }
                uart_putchar('\r');
                uart_putchar('\n');
                
            }
            
            else {
                uart_putchar('?');
                uart_putchar(' ');
                for (uint8_t i = 0; i < cmd_index; i++) {
                    uart_putchar(cmd_buffer[i]);
                }
                uart_putchar('\r');
                uart_putchar('\n');
                uart_putchar('T');
                uart_putchar('r');
                uart_putchar('y');
                uart_putchar(':');
                uart_putchar(' ');
                uart_putchar('e');
                uart_putchar('c');
                uart_putchar('h');
                uart_putchar('o');
                uart_putchar(',');
                uart_putchar(' ');
                uart_putchar('p');
                uart_putchar('s');
                uart_putchar('\r');
                uart_putchar('\n');
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
    //dma_init();
    extern_memory_init();
    //ch375_init();
    __delay_ms(3000);


    ADCON1 = 0xF;
    timer0_init();

    init_scheduler();
    /*
    create_process(&task3, 10);  // this will fail to create process
    create_process(&task1, 2);
    create_process(&task2, 2);
    create_process(&task1, 1);
    create_process(&task1, 2);
    create_process(&task2, 0);
    create_process(&task2, 0);

    create_process(&task3, 0);
    create_process(&task4, 0);
    
    create_process(&shell, 2);
    create_process(&shell, 1);
    create_process(&shell, 0);
    */
    
    
    create_process(&task4, 2);
    
    start_schedule();
    
    PANIC("hello\n");
}
