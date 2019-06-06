//
// Created by Stanislav on 2019-04-26.
//

#ifndef ZLVM_C_INSTRUCTION_H
#define ZLVM_C_INSTRUCTION_H

#include "Condition.h"
#include "Opcode.h"

typedef struct Instruction {
    Opcode opcode_ : 8;
    Condition condition_ : 8;
    byte register1 : 8;
    byte register2 : 8;
    word immediate : 32;
} Instruction;

void instruction_print(Instruction*);

#endif //ZLVM_C_INSTRUCTION_H
