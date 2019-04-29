//
// Created by Stanislav on 2019-04-26.
//

#ifndef ZLVM_C_VIRTUALMACHINE_H
#define ZLVM_C_VIRTUALMACHINE_H

#include "Common.h"
#include "Value.h"
#include "CPSR.h"
#include "Instruction.h"
#include "ALU.h"

static const size_t __ZLVM_MEMORY_SIZE = 1024;
static const size_t __ZLVM_STACK_SIZE = 256;
static const size_t __ZLVM_REGISTER_COUNT = 16;
static const size_t __ZLVM_MACHINE_WORD = sizeof(dword);

struct VirtualMachine {
    union Value _pc;
    union Value _sp;
    union Value _bp;
    union Value _sc;
    union Value _lp;
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

byte readByte(struct VirtualMachine*, size_t address);
void writeByte(struct VirtualMachine*, size_t address, byte value);

word readWord(struct VirtualMachine*, size_t address);
void writeWord(struct VirtualMachine*, size_t address, word value);

dword readDword(struct VirtualMachine*, size_t address);
void writeDword(struct VirtualMachine*, size_t address, dword value);

qword readQword(struct VirtualMachine*, size_t address);
void writeQword(struct VirtualMachine*, size_t address, qword value);

dword popDword(struct VirtualMachine*);
void pushDword(struct VirtualMachine*, dword);

bool notError(struct VirtualMachine*);
bool checkState(struct VirtualMachine*, enum State);
void setState(struct VirtualMachine*, enum State);
enum State getState(struct VirtualMachine*);

void doOperation(struct VirtualMachine*, enum Operation, dword, dword);

void interrupt(struct VirtualMachine*, dword code);
void syscall(struct VirtualMachine*);
#endif //ZLVM_C_VIRTUALMACHINE_H
