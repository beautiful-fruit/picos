typedef struct context {
    unsigned char status;
    unsigned char wreg;
    unsigned char bsr;

    // program counter
    unsigned char pcl;
    unsigned char pch;
    unsigned char pcu;

    // return address stack pointer
    unsigned char rasp;
} Context;