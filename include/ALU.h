//
// Created by Stanislav on 2019-04-26.
//

#ifndef ZLVM_C_ALU_H
#define ZLVM_C_ALU_H

#include "Types.h"

typedef enum Operation {
    OP_NOOP = 0,
    OP_ID, OP_ADD, OP_SUB, OP_MUL, OP_DIV,
    OP_MOD, OP_NOT, OP_AND, OP_OR, OP_XOR,
    OP_NAND, OP_NOR, OP_INC, OP_DEC, OP_SADD,
    OP_SSUB, OP_SMUL, OP_SDIV, OP_SMOD,
    OP_TOTAL,
} Operation;

typedef struct ALUFlags {
    bool N : 1;
    bool Z : 1;
    bool V : 1;
    bool C : 1;
    bool S : 1;
    byte _reserved : 3;
} ALUFlags;

typedef struct ALU {
    word left_;
    word right_;
    union {
        dword result_;
        struct {
            word hi;
            word lo;
        };
    };
    Operation op_;
    ALUFlags flags_;
} ALU;

void alu_setFlags(ALU*);
void alu_compute(ALU*);
void alu_reset(ALU*);

#endif //ZLVM_C_ALU_H
