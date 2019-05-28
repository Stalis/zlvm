//
// Created by Stanislav on 2019-05-27.
//

#ifndef ZLVM_C_VIRTUALMACHINE_INTERNAL_H
#define ZLVM_C_VIRTUALMACHINE_INTERNAL_H

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

#endif //ZLVM_C_VIRTUALMACHINE_INTERNAL_H
