//
// Created by Stanislav on 2019-04-26.
//

#ifndef ZLVM_C_VIRTUALMACHINE_H
#define ZLVM_C_VIRTUALMACHINE_H

#include "../src/CPSR.h"
#include "../src/ALU.h"
#include "../src/Value.h"
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

byte fetchByte(struct VirtualMachine*);
struct Instruction fetchInstruction(struct VirtualMachine*);
void runInstruction(struct VirtualMachine*, struct Instruction);
bool checkCondition(struct VirtualMachine*, enum Condition);

byte readByte(struct VirtualMachine* this, size_t address);
void writeByte(struct VirtualMachine* this, size_t address, byte value);

hword readHword(struct VirtualMachine* this, size_t address);
void writeHword(struct VirtualMachine* this, size_t address, hword value);

word readWord(struct VirtualMachine* this, size_t address);
void writeWord(struct VirtualMachine* this, size_t address, word value);

dword readDword(struct VirtualMachine* this, size_t address);
void writeDword(struct VirtualMachine* this, size_t address, dword value);

word popWord(struct VirtualMachine* this);
void pushWord(struct VirtualMachine* this, word value);

bool notError(struct VirtualMachine*);
bool checkState(struct VirtualMachine*, enum State);
void setState(struct VirtualMachine*, enum State);
enum State getState(struct VirtualMachine*);

void doOperation(struct VirtualMachine*, enum Operation, word, word);

void interrupt(struct VirtualMachine*, word code);
void syscall(struct VirtualMachine*);
#endif //ZLVM_C_VIRTUALMACHINE_H
