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

#define MOD 1000000009UL

uint32_t fsstart_param = 0;
uint32_t fsstart_ans=0;


void __attribute__((naked)) task1(void)
{
    while (1);
    exit();
}

void __attribute__((naked)) task2(void)
{
    
}

void __attribute__((naked)) task3(void)
{
    
}

void __attribute__((naked)) putc(char c)
{
    usr_uart_put_char(c);
}

void __attribute__((naked)) fpstart(void)
{
    uint32_t n = fsstart_param;
    uint32_t result = 0;
    
    for (uint32_t i = 2; i <= n; i++) {
        for(uint32_t j = 2; j < i; j++) {
            if(i % j == 0) {
                result--;
                break;
            }
        }
        result++;
    }
    
    // 使用 spin_lock 保護共享變數
    spin_lock_t ll;
    spin_lock(ll);
    fsstart_ans = result;
    spin_unlock(ll);
    
    exit();
}

void __attribute__((naked)) fpend(void)
{
    uint32_t result;
    
    // 使用 spin_lock 讀取結果
    spin_lock_t ll;
    spin_lock(ll);
    result = fsstart_ans;
    spin_unlock(ll);
    
    if (result == 0) {
        char* st = "e\b\brror: no result available or calculation not finished\r\n";
        for(uint8_t i = 0; i < 58; i++) {
            putc(st[i]);
        }
    } else {
        char* st2 = "r\b\besult: ";
        for(uint8_t i = 0; i < 11; i++) {
            putc(st2[i]);
        }
        
        static char num_str[16];
        uint8_t num_len = 0;
        
        if (result == 0) {
            putc('0');
        } else {
            uint32_t temp = result;
            while (temp > 0) {
                num_str[num_len++] = (temp % 10) + '0';
                temp /= 10;
            }
            
            for (int8_t i = num_len - 1; i >= 0; i--) {
                putc(num_str[i]);
            }
        }
        putc('\r');
        putc('\n');
    }
    
    // 使用 spin_lock 重置結果
    spin_lock_t ll2;
    spin_lock(ll2);
    fsstart_ans = 0;
    spin_unlock(ll2);
    
    exit();
}

void __attribute__((naked)) task4(void)
{
    static char cmd_buffer[32];
    static uint8_t cmd_index = 0;
    
    while(1) {
        lock();
        
        
        putc('$');
        putc(' ');
        
        
        cmd_index = 0;
        while(1) {
            char ch;
            usr_uart_get_char(ch);
            
            
            if (ch == '\r' || ch == '\n') {
                putc('\r');
                putc('\n');
                cmd_buffer[cmd_index] = '\0'; 
                break;
            }
            
            else if (ch == '\b' || ch == 0x7F) {
                if (cmd_index > 0) {
                    cmd_index--;
                    putc('\b');
                    putc(' ');
                    putc('\b');
                }
            }
            
            else if (ch >= 32 && ch <= 126 && cmd_index < 31) {
                putc(ch);  
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
                        putc(cmd_buffer[i]);
                    }
                    putc('\r');
                    putc('\n');
                }
                
                else if (cmd_index == 4) {
                    
                   char* str = "Usage: echo <text>\r\n";
                   for(uint8_t i=0;i<20;i++)
                   {putc(str[i]);}
                }
            }
            
            else if (cmd_index == 2 && 
                     cmd_buffer[0] == 'p' && 
                     cmd_buffer[1] == 's') {
                
                char* str="PID State Stack\r\n------------------\r\n";
                for(uint8_t i=0;i<38;i++)
                {
                    putc(str[i]);
                }
                
                for (uint8_t i = 0; i < 4; i++) {
                    if (run_task_info & (1 << i)) {
                        
                        putc('0' + i);
                        putc(' ');
                        putc(' ');
                        
                        
                        if ((run_task_info >> 4) & 0x3 == i) {
                            putc('R');  // Running
                        } else if (wait_task_info & (1 << i)) {
                            putc('W');  // Waiting
                        } else {
                            putc('S');  // Sleeping/Ready
                        }
                        
                        
                        if (run_task_info & RUN_TASK_EXIT) {
                            putc('X');  
                        } else {
                            putc(' ');
                        }
                        
                        putc(' ');
                        putc(' ');
                        
                        
                        if (run_task[i].stack_info.stack_size > 0) {
                            putc('0' + run_task[i].stack_info.stack_start);
                            putc('-');
                            putc('0' + (run_task[i].stack_info.stack_start + 
                                               run_task[i].stack_info.stack_size - 1));
                        } else {
                            putc('N');
                            putc('/');
                            putc('A');
                        }
                        
                        putc('\r');
                        putc('\n');
                    }
                }
                
                
                str = "\r\nStack use: ";
                for(uint8_t i=0;i<14;i++)
                {
                    putc(str[i]);
                }
                
                for (uint8_t i = 0; i < 4; i++) {
                    if (stack_status.use & (1 << i)) {
                        putc('1');
                    } else {
                        putc('0');
                    }
                }
                putc('\r');
                putc('\n');
                
            }

            else if ( 
                cmd_buffer[0] == 'f' && 
                cmd_buffer[1] == 'p' &&
                cmd_buffer[2] == 's' &&
                cmd_buffer[3] == 't' &&
                cmd_buffer[4] == 'a' &&
                cmd_buffer[5] == 'r' &&
                cmd_buffer[6] == 't' 
                )
            {
                
                uint32_t ans = 0;
                uint8_t has_param = 0;
                
                
                if (cmd_index >= 8 && cmd_buffer[7] == ' ') {
                    has_param = 1;
                    // 讀取參數
                    uint8_t param_index = 8;
                    while (param_index < cmd_index) {
                        if (cmd_buffer[param_index] >= '0' && cmd_buffer[param_index] <= '9') {
                            ans = ans * 10 + (cmd_buffer[param_index] - '0');
                        } else {
                            // 非法字符
                            has_param = 0;
                            break;
                        }
                        param_index++;
                    }
                }
                
                if (has_param) {
                    // 有參數，傳遞給 fsstart
                    spin_lock_t ll;
                    spin_lock(ll);
                    fsstart_param = ans;
                    create_process(&fpstart, 0);
                    spin_unlock(ll);
                } else {
                    // 沒有參數，顯示錯誤訊息
                    char* str = "Error: fpstart requires a number parameter\r\n";
                    for(uint8_t i=0; i<44; i++) {
                        putc(str[i]);
                    }
                }
            }

            else if (cmd_index == 5 && 
                    cmd_buffer[0] == 'f' && 
                    cmd_buffer[1] == 'p' &&
                    cmd_buffer[2] == 'e' &&
                    cmd_buffer[3] == 'n' &&
                    cmd_buffer[4] == 'd' 
                    )
            {
                spin_lock_t ll;
                spin_lock(ll);
                create_process(&fpend,0);
                spin_unlock(ll);
                
            }

            else if (
                    cmd_buffer[0] == 'k' && 
                    cmd_buffer[1] == 'i' &&
                    cmd_buffer[2] == 'l' &&
                    cmd_buffer[3] == 'l' 
                    )
            {
                // 檢查是否有參數
                uint8_t pid = 0;
                uint8_t has_param = 0;
                
                if (cmd_index >= 5 && cmd_buffer[4] == ' ') {
                    has_param = 1;
                    // 讀取參數
                    uint8_t param_index = 5;
                    while (param_index < cmd_index) {
                        if (cmd_buffer[param_index] >= '0' && cmd_buffer[param_index] <= '9') {
                            pid = pid * 10 + (cmd_buffer[param_index] - '0');
                        } else {
                            // 非法字符
                            has_param = 0;
                            break;
                        }
                        param_index++;
                    }
                }
                
                if (has_param && pid < RUN_TASK_SIZE) {  // PID 範圍是 0-3
                    // 檢查該 PID 是否存在
                    if (run_task_info & (1 << pid)) {
                        // 標記該任務為退出狀態
                        spin_lock_t ll;
                        spin_lock(ll);
                        
                        // 設置退出標誌 - 注意：根據你的宏定義，RUN_TASK_EXIT 是 0x40
                        // 需要左移對應的位數來標記特定任務
                        //run_task_info |= RUN_TASK_EXIT;
                        
                        // 釋放該任務的堆疊資源
                        if (run_task[pid].stack_info.stack_size > 0) {
                            // 使用 stack_release 函數釋放堆疊
                            stack_release(pid);
                        }
                        
                        // 清除任務佔用標誌
                        run_task_info &= (uint8_t)~(1 << pid);
                        
                        spin_unlock(ll);
                        
                        char* str = "Process killed successfully\r\n";
                        for(uint8_t i = 0; i < 30; i++) {
                            putc(str[i]);
                        }
                    } else {
                        char* str = "Error: Process does not exist\r\n";
                        for(uint8_t i = 0; i < 32; i++) {
                            putc(str[i]);
                        }
                    }
                } else if (!has_param) {
                    char* str = "Error: kill requires a PID parameter\r\n";
                    for(uint8_t i = 0; i < 38; i++) {
                        putc(str[i]);
                    }
                } else {
                    char str[40];
                    /*
                    str[0] = 'E'; str[1] = 'r'; str[2] = 'r'; str[3] = 'o';
                    str[4] = 'r'; str[5] = ':'; str[6] = ' '; str[7] = 'I';
                    str[8] = 'n'; str[9] = 'v'; str[10] = 'a'; str[11] = 'l';
                    str[12] = 'i'; str[13] = 'd'; str[14] = ' '; str[15] = 'P';
                    str[16] = 'I'; str[17] = 'D'; str[18] = ' '; str[19] = '(';
                    str[20] = 'm'; str[21] = 'u'; str[22] = 's'; str[23] = 't';
                    str[24] = ' '; str[25] = 'b'; str[26] = 'e'; str[27] = ' ';
                    str[28] = '0'; str[29] = '-'; str[30] = '0' + (RUN_TASK_SIZE - 1);
                    str[31] = ')'; str[32] = '\r'; str[33] = '\n'; str[34] = '\0';
                    */
                    char* st = "Error: Invaild PID (must be 0-3)\r\n";
                    for(uint8_t i = 0; i<34; i++) {
                        putc(st[i]);
                    }
                }
            }
            
            else {
                putc('?');
                putc(' ');
                for (uint8_t i = 0; i < cmd_index; i++) {
                    putc(cmd_buffer[i]);
                }
                
               char* str = "\r\nTry: echo, ps, fpstart, fpend, kill\r\n";
               for(uint8_t i=0;i<39;i++)
               {
                    putc(str[i]);
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
    //dma_init();
    extern_memory_init();
    ch375_init();
    __delay_ms(3000);
    INTCONbits.GIE = 1;

    ADCON1 = 0xF;
    timer0_init();

    init_scheduler();    
    
    create_process(&task4, 2);
    
    
    start_schedule();
    
    PANIC("hello\n");
}