// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
// Created by Stanislav on 2019-05-07.
//

#include "Statement.h"

void statement_init(struct Statement* s) {
    s->opcode = NULL;
    s->cond = NULL;
    s->reg1 = NULL;
    s->reg2 = NULL;
    s->imm = NULL;
}