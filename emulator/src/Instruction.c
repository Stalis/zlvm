//
// Created by Stanislav on 2019-06-06.
//

#include "Instruction.h"

#include <stdio.h>

_Static_assert(OPCODE_TOTAL - 1 <= UINT8_MAX, "Opcode must fit in one byte");
_Static_assert(C_TOTAL - 1 <= UINT8_MAX, "Condition must fit in one byte");

bool instruction_encode(byte *output, size_t output_size, const Instruction *instruction) {
    if (output == NULL || output_size < ZLVM_INSTRUCTION_SIZE || instruction == NULL ||
        (unsigned)instruction->opcode_ >= OPCODE_TOTAL ||
        (unsigned)instruction->condition_ >= C_TOTAL) {
        return false;
    }

    output[0] = (byte)instruction->opcode_;
    output[1] = (byte)instruction->condition_;
    output[2] = instruction->register1;
    output[3] = instruction->register2;
    for (size_t index = 0; index < sizeof instruction->immediate; index++) {
        output[4 + index] = (byte)(instruction->immediate >> (index * 8));
    }
    return true;
}

bool instruction_decode(Instruction *output, const byte *input, size_t input_size) {
    if (output == NULL || input == NULL || input_size < ZLVM_INSTRUCTION_SIZE) {
        return false;
    }

    Instruction instruction = {
        .opcode_ = (Opcode)input[0],
        .condition_ = (Condition)input[1],
        .register1 = input[2],
        .register2 = input[3],
    };
    for (size_t index = 0; index < sizeof instruction.immediate; index++) {
        instruction.immediate |= (word)input[4 + index] << (index * 8);
    }
    *output = instruction;
    return true;
}

void instruction_print(Instruction *instruction) {
    const char *opcode = opcode_to_string(instruction->opcode_);
    const char *condition = condition_to_string(instruction->condition_);
    printf("%s %s %d, %d, 0x%X\n", opcode, condition, instruction->register1,
           instruction->register2, instruction->immediate);
}
