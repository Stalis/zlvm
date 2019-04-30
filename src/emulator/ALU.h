//
// Created by Stanislav on 2019-04-26.
//

#ifndef ZLVM_C_ALU_H
#define ZLVM_C_ALU_H

#include "../common/Types.h"

enum Operation {
    OP_NOOP = 0,
    OP_ID, OP_ADD, OP_SUB, OP_MUL, OP_DIV,
    OP_MOD, OP_NOT, OP_AND, OP_OR, OP_XOR,
    OP_NAND, OP_NOR, OP_INC, OP_DEC, OP_SADD,
    OP_SSUB, OP_SMUL, OP_SDIV, OP_SMOD,
    OP_TOTAL,
};

struct ALUFlags {
    bool N : 1;
    bool Z : 1;
    bool V : 1;
    bool C : 1;
    bool S : 1;
    byte _reserved : 3;
};

struct ALU {
    word left_;
    word right_;
    word result_;
    enum Operation op_;
    struct ALUFlags flags_;
};

void alu_setFlags(struct ALU*);
void alu_compute(struct ALU*);
void alu_reset(struct ALU*);

#endif //ZLVM_C_ALU_H
