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

static const size_t __ZLVM_ROM_SIZE = 1024;
static const size_t __ZLVM_STACK_SIZE = 256;
static const size_t __ZLVM_REGISTER_COUNT = R_TOTAL;
static const size_t __ZLVM_WORD_SIZE = sizeof(word);

typedef struct VirtualMachine {
    CPSR _cpsr;
    ALU _alu;
    Value _registers[__ZLVM_REGISTER_COUNT];
    byte _rom[__ZLVM_ROM_SIZE];
    byte* _memory;
    size_t _memorySize;
} VirtualMachine;

/**
 * @brief Initialize virtual machine instance
 */
void vm_initialize(VirtualMachine* vm, size_t ram_size);

/**
 * @brief Load dump to vm's memory
 * @param program pointer to byte array of dump
 * @param size size of dump
 */
void vm_loadDump(VirtualMachine* vm, const byte* program, size_t size);

/**
 * @brief Start vm
 * @return exit state of vm
 */
State vm_run(VirtualMachine* vm);

#endif //ZLVM_C_VIRTUALMACHINE_H
