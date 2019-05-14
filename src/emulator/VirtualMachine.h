//
// Created by Stanislav on 2019-04-26.
//

#ifndef ZLVM_C_VIRTUALMACHINE_H
#define ZLVM_C_VIRTUALMACHINE_H

#include "../common/Types.h"
#include "../common/Value.h"
#include "CPSR.h"
#include "../common/Instruction.h"
#include "ALU.h"
#include "../common/Registers.h"

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

void initialize(struct VirtualMachine*);
void loadDump(struct VirtualMachine*, byte* program, size_t size);
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
