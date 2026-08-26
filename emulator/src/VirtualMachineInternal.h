//
// Created by Stanislav on 2019-05-27.
//

#ifndef ZLVM_EMULATOR_VIRTUAL_MACHINE_INTERNAL_H
#define ZLVM_EMULATOR_VIRTUAL_MACHINE_INTERNAL_H

#include "VirtualMachine.h"

byte vm_fetch_byte(VirtualMachine *);
Instruction vm_fetch_instruction(VirtualMachine *);
void vm_run_instruction(VirtualMachine *, Instruction);
bool vm_check_condition(VirtualMachine *, Condition);

byte vm_read_byte(VirtualMachine *vm, size_t address);
void vm_write_byte(VirtualMachine *vm, size_t address, byte value);

hword vm_read_hword(struct VirtualMachine *vm, size_t address);
void vm_write_hword(struct VirtualMachine *vm, size_t address, hword value);

word vm_read_word(struct VirtualMachine *vm, size_t address);
void vm_write_word(struct VirtualMachine *vm, size_t address, word value);

dword vm_read_dword(struct VirtualMachine *vm, size_t address);
void vm_write_dword(struct VirtualMachine *vm, size_t address, dword value);

bool vm_pop_word(struct VirtualMachine *vm, word *value);
bool vm_push_word(struct VirtualMachine *vm, word value);

bool vm_has_no_error(struct VirtualMachine *);
bool vm_has_state(struct VirtualMachine *, enum State);
void vm_set_state(struct VirtualMachine *, enum State);
enum State vm_get_state(struct VirtualMachine *);

void vm_do_operation(struct VirtualMachine *, enum Operation, word, word);

void vm_interrupt(struct VirtualMachine *, word code);
void vm_syscall(struct VirtualMachine *);

#endif // ZLVM_EMULATOR_VIRTUAL_MACHINE_INTERNAL_H
