#pragma once
#include <xc.h>

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

#define USTACK_SIZE 256

typedef struct task {
    char *sp;
    char stack[USTACK_SIZE];
    Context context;
} Task;

typedef void (*func_t)(void);