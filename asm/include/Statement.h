//
// Created by Stanislav on 2019-05-07.
//

#ifndef ZLVM_C_STATEMENT_H
#define ZLVM_C_STATEMENT_H

#include "../../emulator/include/Opcode.h"
#include "../../emulator/include/Condition.h"
#include "Token.h"

struct Statement {
    struct Token* opcode;
    struct Token* cond;
    struct Token* reg1;
    struct Token* reg2;
    struct Token* imm;
};

void statement_init(struct Statement*);

#endif //ZLVM_C_STATEMENT_H
