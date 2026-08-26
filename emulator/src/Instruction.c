//
// Created by Stanislav on 2019-06-06.
//

#include "Instruction.h"

#include <stdio.h>

void instruction_print(Instruction *instruction) {
    const char *opcode = opcode_to_string(instruction->opcode_);
    const char *condition = condition_to_string(instruction->condition_);
    printf("%s %s %d, %d, 0x%X\n", opcode, condition, instruction->register1,
           instruction->register2, instruction->immediate);
}
