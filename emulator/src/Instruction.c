// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Created by Stanislav on 2019-06-06.
//

#include "../../include/Instruction.h"
#include <printf.h>

void instruction_print(Instruction* i) {
    const char* opcode = opcode_to_string(i->opcode_);
    const char* cond = condition_to_string(i->condition_);
    printf("%s %s %d, %d, 0x%X\n", opcode, cond, i->register1, i->register2, i->immediate);
}