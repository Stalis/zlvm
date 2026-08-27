//
// Created by Stanislav on 2019-06-06.
//

#include "Instruction.h"

#include <stdio.h>

void instruction_encode(byte output[ZLVM_INSTRUCTION_SIZE], const Instruction *instruction) {
    output[0] = (byte)instruction->opcode_;
    output[1] = (byte)instruction->condition_;
    output[2] = instruction->register1;
    output[3] = instruction->register2;
    for (size_t index = 0; index < sizeof instruction->immediate; index++) {
        output[4 + index] = (byte)(instruction->immediate >> (index * 8));
    }
}

Instruction instruction_decode(const byte input[ZLVM_INSTRUCTION_SIZE]) {
    Instruction instruction = {
        .opcode_ = (Opcode)input[0],
        .condition_ = (Condition)input[1],
        .register1 = input[2],
        .register2 = input[3],
    };
    for (size_t index = 0; index < sizeof instruction.immediate; index++) {
        instruction.immediate |= (word)input[4 + index] << (index * 8);
    }
    return instruction;
}

void instruction_print(Instruction *instruction) {
    const char *opcode = opcode_to_string(instruction->opcode_);
    const char *condition = condition_to_string(instruction->condition_);
    printf("%s %s %d, %d, 0x%X\n", opcode, condition, instruction->register1,
           instruction->register2, instruction->immediate);
}
