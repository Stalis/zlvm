// Created by Stanislav on 2019-05-07.
//

#include "Statement.h"

void statement_init(Statement *statement) {
    statement->opcode = NULL;
    statement->cond = NULL;
    statement->reg1 = NULL;
    statement->reg2 = NULL;
    statement->imm = NULL;
}
