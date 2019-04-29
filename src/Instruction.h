//
// Created by Stanislav on 2019-04-26.
//

#ifndef ZLVM_C_INSTRUCTION_H
#define ZLVM_C_INSTRUCTION_H

#include "Condition.h"
#include "Opcode.h"

struct Instruction {
    enum Opcode opcode_ : 8;
    enum Condition condition_ : 8;
    byte register1 : 8;
    byte register2 : 8;
    dword immediate : 32;
};

#endif //ZLVM_C_INSTRUCTION_H
