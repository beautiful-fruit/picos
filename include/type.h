#pragma once
#ifdef __XC8
#include <xc.h>
#else
#include "dfp/xc8/pic/include/proc/pic18f4520.h"
#define __uint24 uint32_t
#endif


typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;
typedef unsigned long addr_t;

union int24_part {
    __uint24 value;
    struct {
        unsigned char l;
        unsigned char h;
        unsigned char u;
    };
};

typedef struct context {
    unsigned char status;
    unsigned char wreg;
    unsigned char bsr;

    // program counter
    union int24_part pc;

    // return address stack pointer
    unsigned char rasp;

    unsigned short fsr0, fsr1, fsr2;

    unsigned short prod;

    unsigned char tablat;
    unsigned char tblptrl, tblptrh, tblptru;
    unsigned char eecon1;
} Context;

#define USTACK_SIZE 128

typedef struct task {
    uint8_t *sp;
    Context context;
    union {
        struct {
            unsigned stack_start : 4;
            // stack_size indicates that the stack size is 128 multiplied by
            // stack_size
            unsigned stack_size : 4;
        };
    } stack_info;
} Task;

typedef void (*func_t)(void);

typedef struct {
    func_t func;
    uint8_t stack_size;
} wait_node_t;

typedef union {
    struct {
        unsigned nop : 1;
        unsigned wait_pid : 3;
        unsigned use : 4;
    };
} stack_status_t;
