//
// Created by Stanislav on 2019-05-07.
//

#include "../include/Statement.h"

void statement_init(struct Statement* s) {
    s->opcode = NULL;
    s->cond = NULL;
    s->reg1 = NULL;
    s->reg2 = NULL;
    s->imm = NULL;
}