//
// Created by Stanislav on 2019-05-07.
//

#ifndef ZLVM_C_STATEMENT_H
#define ZLVM_C_STATEMENT_H

#include "../../emulator/include/Opcode.h"
#include "../../emulator/include/Condition.h"
#include "Token.h"

typedef struct Statement {
    Token* opcode;
    Token* cond;
    Token* reg1;
    Token* reg2;
    Token* imm;
} Statement;

void statement_init(Statement*);

#endif //ZLVM_C_STATEMENT_H
