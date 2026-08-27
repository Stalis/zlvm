//
// Created by Stanislav on 2019-04-26.
//

#ifndef ZLVM_C_INSTRUCTION_H
#define ZLVM_C_INSTRUCTION_H

#include "Condition.h"
#include "Opcode.h"

typedef struct Instruction {
    Opcode opcode_;
    Condition condition_;
    byte register1;
    byte register2;
    word immediate;
} Instruction;

enum { ZLVM_INSTRUCTION_SIZE = 8 };

/**
 * Encodes an instruction into the first ZLVM_INSTRUCTION_SIZE bytes of output.
 * Returns false without modifying output if a pointer is null, output is too small, or an enum
 * field cannot be encoded.
 */
bool instruction_encode(byte *output, size_t output_size, const Instruction *instruction);

/**
 * Decodes the first ZLVM_INSTRUCTION_SIZE bytes of input into output.
 * Returns false without modifying output if a pointer is null or input is too small. Encoded field
 * values are not semantically validated.
 */
bool instruction_decode(Instruction *output, const byte *input, size_t input_size);
void instruction_print(Instruction *instruction);

#endif // ZLVM_C_INSTRUCTION_H
