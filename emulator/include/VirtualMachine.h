//
// Created by Stanislav on 2019-04-26.
//

#ifndef ZLVM_C_VIRTUALMACHINE_H
#define ZLVM_C_VIRTUALMACHINE_H

#include "ALU.h"
#include "CPSR.h"
#include "Value.h"
#include "Registers.h"
#include "Types.h"
#include "Condition.h"
#include "Instruction.h"

static const size_t __ZLVM_MEMORY_SIZE = 1024;
static const size_t __ZLVM_STACK_SIZE = 256;
static const size_t __ZLVM_REGISTER_COUNT = R_TOTAL;
static const size_t __ZLVM_WORD_SIZE = sizeof(word);

struct VirtualMachine {
    union CPSR _cpsr;
    struct ALU _alu;
    union Value _registers[__ZLVM_REGISTER_COUNT];
    byte _memory[__ZLVM_MEMORY_SIZE + __ZLVM_STACK_SIZE];
};

/**
 * @brief Initialize virtual machine instance
 */
void initialize(struct VirtualMachine*);

/**
 * @brief Load dump to vm's memory
 * @param program pointer to byte array of dump
 * @param size size of dump
 */
void loadDump(struct VirtualMachine*, byte* program, size_t size);

/**
 * @brief Start vm
 * @return exit state of vm
 */
enum State run(struct VirtualMachine*);

#endif //ZLVM_C_VIRTUALMACHINE_H
