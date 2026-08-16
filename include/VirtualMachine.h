//
// Created by Stanislav on 2019-04-26.
//

#ifndef ZLVM_C_VIRTUALMACHINE_H
#define ZLVM_C_VIRTUALMACHINE_H

#include "ALU.h"
#include "CPSR.h"
#include "Condition.h"
#include "Instruction.h"
#include "Registers.h"
#include "Types.h"
#include "Value.h"

enum {
    ZLVM_ROM_SIZE = 4096,
    ZLVM_STACK_SIZE = 256,
    ZLVM_REGISTER_COUNT = R_TOTAL,
    ZLVM_WORD_SIZE = sizeof(word),
};

typedef struct VirtualMachine {
    CPSR _cpsr;
    ALU _alu;
    Value _registers[ZLVM_REGISTER_COUNT];
    byte _rom[ZLVM_ROM_SIZE];
    byte* _memory;
    size_t _memorySize;
} VirtualMachine;

/**
 * @brief Initialize virtual machine instance
 */
void vm_initialize(VirtualMachine* vm, size_t ram_size);

/**
 * @brief Release memory owned by a virtual machine instance.
 */
void vm_destroy(VirtualMachine* vm);

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

#endif // ZLVM_C_VIRTUALMACHINE_H
