//
// Created by Stanislav on 2019-04-26.
//

#ifndef ZLVM_C_INSTRUCTION_H
#define ZLVM_C_INSTRUCTION_H

#include "Condition.h"
#include "Opcode.h"

typedef struct Instruction {
    Opcode opcode_;
    Condition condition_;
    byte register1;
    byte register2;
    word immediate;
} Instruction;

enum { ZLVM_INSTRUCTION_SIZE = 8 };

void instruction_encode(byte output[ZLVM_INSTRUCTION_SIZE], const Instruction *instruction);
Instruction instruction_decode(const byte input[ZLVM_INSTRUCTION_SIZE]);
void instruction_print(Instruction *instruction);

#endif // ZLVM_C_INSTRUCTION_H
