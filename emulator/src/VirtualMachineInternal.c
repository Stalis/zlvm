// Created by Stanislav on 2019-04-26.
//
#include "VirtualMachineInternal.h"

#include "Memory.h"

#include <stdio.h>
#include <string.h>

static bool vm_validate_range(VirtualMachine *vm, size_t address, size_t size);
static bool vm_effective_address(VirtualMachine *vm, word base, word offset, size_t size,
                                 size_t *address);
static dword vm_read_scalar(VirtualMachine *vm, size_t address, size_t size);
static void vm_write_scalar(VirtualMachine *vm, size_t address, dword value, size_t size);

#if DEBUG

static inline void print_registers(VirtualMachine *vm, byte columns) {
    for (word i = 0; i < ZLVM_REGISTER_COUNT; i += columns) {
        for (byte j = 0; j < columns && (i + j) < ZLVM_REGISTER_COUNT; j++) {
            printf("[r%d]:\t%d\t", i + j, vm->_registers[i + j].word_);
        }
        printf("\n");
    }

    printf("[PC]:\t%d\t[SP]:\t%d\t[BP]:\t%d\t[SC]:\t%d\t[LP]:\t%d\n", vm->_registers[R_PC].word_,
           vm->_registers[R_SP].word_, vm->_registers[R_BP].word_, vm->_registers[R_SC].word_,
           vm->_registers[R_LP].word_);
    printf("[CPSR]: N:%d Z:%d V:%d C:%d S:%d ST:%d\n", vm->_cpsr.N, vm->_cpsr.Z, vm->_cpsr.V,
           vm->_cpsr.C, vm->_cpsr.S, vm->_cpsr.ST);
}
#endif

void vm_initialize(VirtualMachine *vm, size_t ram_size) {
    for (size_t i = 0; i < ZLVM_REGISTER_COUNT; i++) {
        vm->_registers[i].word_ = 0;
    }
    for (size_t i = 0; i < ZLVM_ROM_SIZE; i++) {
        vm->_rom[i] = 0;
    }

    vm->_memorySize = ram_size;
    vm->_memory = vm_calloc(ram_size, sizeof *vm->_memory);

    vm->_cpsr.value_.word_ = 0;
    alu_reset(&vm->_alu);
    vm_set_state(vm, S_NORMAL);
}

void vm_destroy(VirtualMachine *vm) {
    if (vm == NULL) {
        return;
    }
    free(vm->_memory);
    vm->_memory = NULL;
    vm->_memorySize = 0;
}

void vm_loadDump(VirtualMachine *vm, const byte *program, size_t size) {
    if (size > ZLVM_ROM_SIZE) {
        vm_set_state(vm, S_ERR_OUT_OF_MEMORY);
        return;
    }
    for (size_t i = 0; i < size; i++) {
        vm->_rom[i] = program[i];
    }
}

State vm_run(VirtualMachine *vm) {
    vm->_registers[R_PC].word_ = 0; // set to start of rom
    vm->_registers[R_BP].word_ = ZLVM_ROM_SIZE;
    vm->_registers[R_SP].word_ = vm->_registers[R_BP].word_; // set to start of memory
    while (vm_has_no_error(vm) && !vm_has_state(vm, S_HALTED)) {
        Instruction instruction = vm_fetch_instruction(vm);
        if (vm_has_no_error(vm)) {
            vm_run_instruction(vm, instruction);
        }
    }
#if DEBUG
    print_registers(vm, 4);
#endif
    return vm_get_state(vm);
}

byte vm_fetch_byte(VirtualMachine *vm) {
    return vm_read_byte(vm, vm->_registers[R_PC].word_++);
}

Instruction vm_fetch_instruction(VirtualMachine *vm) {
    size_t address = vm->_registers[R_PC].word_;
    if (!vm_validate_range(vm, address, ZLVM_INSTRUCTION_SIZE)) {
        return (Instruction){0};
    }

    byte bytes[ZLVM_INSTRUCTION_SIZE];
    for (size_t index = 0; index < ZLVM_INSTRUCTION_SIZE; index++) {
        bytes[index] = vm_fetch_byte(vm);
    }
    return instruction_decode(bytes);
}

void vm_run_instruction(VirtualMachine *vm, Instruction instruction) {
    vm->_registers[R_ZERO].word_ = 0;

    if (!vm_check_condition(vm, instruction.condition_)) {
        return;
    }

    if (instruction.register1 >= ZLVM_REGISTER_COUNT ||
        instruction.register2 >= ZLVM_REGISTER_COUNT) {
        vm_set_state(vm, S_ERR_INVALID_OPCODE);
        return;
    }

    Value *reg1 = &vm->_registers[instruction.register1];
    Value *reg2 = &vm->_registers[instruction.register2];
    word imm = instruction.immediate;
    bool write_alu_result = false;

#ifdef DEBUG
    /*
    printf("[S: 0x%X LP: 0x%X]: ", vm_read_word(vm, vm->_registers[R_SP].word_),
           vm->_registers[R_LP].word_);
    printf("0x%X: ", vm->_registers[R_PC].word_);
    instruction_print(&instruction);
     */
#endif

    switch (instruction.opcode_) {
        case NOP:
            break;

        case POPR: {
            word value;
            if (vm_pop_word(vm, &value)) {
                reg1->word_ = value;
            }
        } break;
        case POP:
            vm_pop_word(vm, NULL);
            break;

        case PUSHR:
            vm_push_word(vm, reg1->word_);
            break;
        case PUSHI:
            vm_push_word(vm, imm);
            break;
        case DUP: {
            word value;
            if (vm_pop_word(vm, &value)) {
                vm_push_word(vm, value);
                vm_push_word(vm, value);
            }
        } break;

        case MOVR:
            reg1->word_ = reg2->word_;
            break;
        case MOVI:
            reg1->word_ = imm;
            break;

        case ADDR:
            vm_do_operation(vm, OP_ADD, reg1->word_, reg2->word_);
            write_alu_result = true;
            break;
        case SUBR:
            vm_do_operation(vm, OP_SUB, reg1->word_, reg2->word_);
            write_alu_result = true;
            break;
        case MULR:
            vm_do_operation(vm, OP_MUL, reg1->word_, reg2->word_);
            write_alu_result = true;
            break;
        case DIVR:
            vm_do_operation(vm, OP_DIV, reg1->word_, reg2->word_);
            write_alu_result = true;
            break;
        case MODR:
            vm_do_operation(vm, OP_MOD, reg1->word_, reg2->word_);
            write_alu_result = true;
            break;

        case ADDI:
            vm_do_operation(vm, OP_ADD, reg1->word_, imm);
            write_alu_result = true;
            break;
        case SUBI:
            vm_do_operation(vm, OP_SUB, reg1->word_, imm);
            write_alu_result = true;
            break;
        case MULI:
            vm_do_operation(vm, OP_MUL, reg1->word_, imm);
            write_alu_result = true;
            break;
        case DIVI:
            vm_do_operation(vm, OP_DIV, reg1->word_, imm);
            write_alu_result = true;
            break;
        case MODI:
            vm_do_operation(vm, OP_MOD, reg1->word_, imm);
            write_alu_result = true;
            break;

        case ADDSR:
            vm_do_operation(vm, OP_SADD, reg1->word_, reg2->word_);
            write_alu_result = true;
            break;
        case SUBSR:
            vm_do_operation(vm, OP_SSUB, reg1->word_, reg2->word_);
            write_alu_result = true;
            break;
        case MULSR:
            vm_do_operation(vm, OP_SMUL, reg1->word_, reg2->word_);
            write_alu_result = true;
            break;
        case DIVSR:
            vm_do_operation(vm, OP_SDIV, reg1->word_, reg2->word_);
            write_alu_result = true;
            break;
        case MODSR:
            vm_do_operation(vm, OP_SMOD, reg1->word_, reg2->word_);
            write_alu_result = true;
            break;

        case ADDSI:
            vm_do_operation(vm, OP_SADD, reg1->word_, imm);
            write_alu_result = true;
            break;
        case SUBSI:
            vm_do_operation(vm, OP_SSUB, reg1->word_, imm);
            write_alu_result = true;
            break;
        case MULSI:
            vm_do_operation(vm, OP_SMUL, reg1->word_, imm);
            write_alu_result = true;
            break;
        case DIVSI:
            vm_do_operation(vm, OP_SDIV, reg1->word_, imm);
            write_alu_result = true;
            break;
        case MODSI:
            vm_do_operation(vm, OP_SMOD, reg1->word_, imm);
            write_alu_result = true;
            break;

        case NOT:
            vm_do_operation(vm, OP_NOT, reg1->word_, 0);
            write_alu_result = true;
            break;
        case ANDR:
            vm_do_operation(vm, OP_AND, reg1->word_, reg2->word_);
            write_alu_result = true;
            break;
        case ORR:
            vm_do_operation(vm, OP_OR, reg1->word_, reg2->word_);
            write_alu_result = true;
            break;
        case XORR:
            vm_do_operation(vm, OP_XOR, reg1->word_, reg2->word_);
            write_alu_result = true;
            break;
        case NANDR:
            vm_do_operation(vm, OP_NAND, reg1->word_, reg2->word_);
            write_alu_result = true;
            break;
        case NORR:
            vm_do_operation(vm, OP_NOR, reg1->word_, reg2->word_);
            write_alu_result = true;
            break;

        case ANDI:
            vm_do_operation(vm, OP_AND, reg1->word_, imm);
            write_alu_result = true;
            break;
        case ORI:
            vm_do_operation(vm, OP_OR, reg1->word_, imm);
            write_alu_result = true;
            break;
        case XORI:
            vm_do_operation(vm, OP_XOR, reg1->word_, imm);
            write_alu_result = true;
            break;
        case NANDI:
            vm_do_operation(vm, OP_NAND, reg1->word_, imm);
            write_alu_result = true;
            break;
        case NORI:
            vm_do_operation(vm, OP_NOR, reg1->word_, imm);
            write_alu_result = true;
            break;

        case INC:
            vm_do_operation(vm, OP_INC, reg1->word_, 0);
            write_alu_result = true;
            break;
        case DEC:
            vm_do_operation(vm, OP_DEC, reg1->word_, 0);
            write_alu_result = true;
            break;

        case LOADB: {
            size_t address;
            if (vm_effective_address(vm, reg2->word_, imm, sizeof(byte), &address)) {
                reg1->word_ = (reg1->word_ & ~((word)BYTE_MAX)) | vm_read_byte(vm, address);
            }
        } break;
        case LOADH: {
            size_t address;
            if (vm_effective_address(vm, reg2->word_, imm, sizeof(hword), &address)) {
                reg1->word_ = (reg1->word_ & ~((word)HWORD_MAX)) | vm_read_hword(vm, address);
            }
        } break;
        case LOADW: {
            size_t address;
            if (vm_effective_address(vm, reg2->word_, imm, sizeof(word), &address)) {
                reg1->word_ = vm_read_word(vm, address);
            }
        } break;

        case STOREB: {
            size_t address;
            if (vm_effective_address(vm, reg2->word_, imm, sizeof(byte), &address)) {
                vm_write_byte(vm, address, (byte)reg1->word_);
            }
        } break;
        case STOREH: {
            size_t address;
            if (vm_effective_address(vm, reg2->word_, imm, sizeof(hword), &address)) {
                vm_write_hword(vm, address, (hword)reg1->word_);
            }
        } break;
        case STOREW: {
            size_t address;
            if (vm_effective_address(vm, reg2->word_, imm, sizeof(word), &address)) {
                vm_write_word(vm, address, reg1->word_);
            }
        } break;

        case INT:
            vm_interrupt(vm, imm);
            break;
        case SYSCALL:
            vm_syscall(vm);
            break;

        case JMPAL:
            vm->_registers[R_LP].word_ = vm->_registers[R_PC].word_;
            vm->_registers[R_PC].word_ = imm;
            break;
        case JMP:
            vm->_registers[R_PC].word_ = imm;
            break;
        case RET:
            vm->_registers[R_PC].word_ = vm->_registers[R_LP].word_;
            break;

        case CMPI:
            vm_do_operation(vm, OP_SUB, reg1->word_, imm);
            break;
        case CMPR:
            vm_do_operation(vm, OP_SUB, reg1->word_, reg2->word_);
            break;
        case CMPSI:
            vm_do_operation(vm, OP_SSUB, reg1->word_, imm);
            break;
        case CMPSR:
            vm_do_operation(vm, OP_SSUB, reg1->word_, reg2->word_);
            break;

        default:
            vm_set_state(vm, S_ERR_INVALID_OPCODE);
            break;
    }

    if (write_alu_result && vm_has_no_error(vm)) {
        reg1->word_ = vm->_alu.result_;
    }

    vm->_registers[R_ZERO].word_ = 0;

    if (vm->_alu.op_ != OP_NOOP) {
        alu_reset(&vm->_alu);
    }
}

bool vm_check_condition(VirtualMachine *vm, Condition condition) {
    switch (condition) {
        case C_UNCONDITIONAL:
            return true;

        case C_ZERO_SET:
            return (vm->_cpsr.Z);
        case C_ZERO_CLEAR:
            return !(vm->_cpsr.Z);

        case C_NEGATIVE_SET:
            return vm->_cpsr.N;
        case C_NEGATIVE_CLEAR:
            return !vm->_cpsr.N;

        case C_OVERFLOW_SET:
            return vm->_cpsr.V;
        case C_OVERFLOW_CLEAR:
            return !vm->_cpsr.V;

        case C_CARRY_SET:
            return vm->_cpsr.C;
        case C_CARRY_CLEAR:
            return !vm->_cpsr.C;

        case C_SIGNED_SET:
            return vm->_cpsr.S;
        case C_SIGNED_CLEAR:
            return !vm->_cpsr.S;

        case C_UNSIGNED_HIGHER:
            return (vm->_cpsr.C && !vm->_cpsr.Z);
        case C_UNSIGNED_LOWER_OR_SAME:
            return (!vm->_cpsr.C || vm->_cpsr.Z);

        case C_LESS:
            return (vm->_cpsr.N) ^ (vm->_cpsr.V);
        case C_LESS_OR_EQUALS:
            return (vm->_cpsr.Z) || ((vm->_cpsr.N) ^ (vm->_cpsr.V));
        case C_GREATER:
            return !((vm->_cpsr.Z) || ((vm->_cpsr.N) ^ (vm->_cpsr.V)));
        case C_GREATER_OR_EQUALS:
            return !((vm->_cpsr.N) ^ (vm->_cpsr.V));

        default:
            vm_set_state(vm, S_ERR_INVALID_CONDITION);
            return false;
    }
}

byte vm_read_byte(VirtualMachine *vm, size_t address) {
    if (!vm_validate_range(vm, address, sizeof(byte))) {
        return 0;
    }

    if (address >= ZLVM_ROM_SIZE) {
        address -= ZLVM_ROM_SIZE;
        return vm->_memory[address];
    }
    return vm->_rom[address];
}

void vm_write_byte(VirtualMachine *vm, size_t address, byte value) {
    if (!vm_validate_range(vm, address, sizeof(byte))) {
        return;
    }

    if (address >= ZLVM_ROM_SIZE) {
        address -= ZLVM_ROM_SIZE;
        vm->_memory[address] = value;
        return;
    }
    vm->_rom[address] = value;
}

hword vm_read_hword(VirtualMachine *vm, size_t address) {
    return (hword)vm_read_scalar(vm, address, sizeof(hword));
}

void vm_write_hword(VirtualMachine *vm, size_t address, hword value) {
    vm_write_scalar(vm, address, value, sizeof value);
}

word vm_read_word(VirtualMachine *vm, size_t address) {
    return (word)vm_read_scalar(vm, address, sizeof(word));
}

void vm_write_word(VirtualMachine *vm, size_t address, word value) {
    vm_write_scalar(vm, address, value, sizeof value);
}

dword vm_read_dword(VirtualMachine *vm, size_t address) {
    return vm_read_scalar(vm, address, sizeof(dword));
}

void vm_write_dword(VirtualMachine *vm, size_t address, dword value) {
    vm_write_scalar(vm, address, value, sizeof value);
}

static dword vm_read_scalar(VirtualMachine *vm, size_t address, size_t size) {
    if (!vm_validate_range(vm, address, size)) {
        return 0;
    }

    dword result = 0;
    for (size_t index = 0; index < size; index++) {
        result |= (dword)vm_read_byte(vm, address + index) << (index * 8);
    }
    return result;
}

static void vm_write_scalar(VirtualMachine *vm, size_t address, dword value, size_t size) {
    if (!vm_validate_range(vm, address, size)) {
        return;
    }

    for (size_t index = 0; index < size; index++) {
        vm_write_byte(vm, address + index, (byte)(value >> (index * 8)));
    }
}

bool vm_pop_word(VirtualMachine *vm, word *value) {
    word stack_pointer = vm->_registers[R_SP].word_;
    word base_pointer = vm->_registers[R_BP].word_;
    word used_stack = stack_pointer - base_pointer;

    if (stack_pointer < base_pointer || used_stack < ZLVM_WORD_SIZE ||
        used_stack > ZLVM_STACK_SIZE || used_stack % ZLVM_WORD_SIZE != 0) {
        vm_set_state(vm, S_ERR_STACK_UNDERFLOW);
        return false;
    }

    size_t address = stack_pointer - ZLVM_WORD_SIZE;
    word result = vm_read_word(vm, address);
    if (!vm_has_no_error(vm)) {
        return false;
    }
    vm->_registers[R_SP].word_ = (word)address;
    if (value != NULL) {
        *value = result;
    }
    return true;
}

bool vm_push_word(VirtualMachine *vm, word value) {
    word stack_pointer = vm->_registers[R_SP].word_;
    word base_pointer = vm->_registers[R_BP].word_;
    word used_stack = stack_pointer - base_pointer;

    if (stack_pointer < base_pointer || used_stack > ZLVM_STACK_SIZE - ZLVM_WORD_SIZE ||
        used_stack % ZLVM_WORD_SIZE != 0) {
        vm_set_state(vm, S_ERR_STACK_OVERFLOW);
        return false;
    }

    size_t address = stack_pointer;
    if (!vm_validate_range(vm, address, sizeof value)) {
        return false;
    }
    vm_write_word(vm, address, value);
    if (!vm_has_no_error(vm)) {
        return false;
    }
    vm->_registers[R_SP].word_ = (word)address + ZLVM_WORD_SIZE;
    return true;
}

bool vm_has_no_error(VirtualMachine *vm) {
    return !is_error(vm->_cpsr.ST);
}

bool vm_has_state(VirtualMachine *vm, State state) {
    return vm->_cpsr.ST == state;
}

void vm_set_state(VirtualMachine *vm, State state) {
    if (vm_has_no_error(vm)) {
        vm->_cpsr.ST = state;
    }
}

enum State vm_get_state(VirtualMachine *vm) {
    return vm->_cpsr.ST;
}

void vm_do_operation(VirtualMachine *vm, Operation op, word left, word right) {
    if ((op == OP_DIV || op == OP_MOD || op == OP_SDIV || op == OP_SMOD) && right == 0) {
        vm_set_state(vm, S_ERR_DIVISION_BY_ZERO);
        return;
    }

    vm->_alu.op_ = op;
    vm->_alu.left_ = left;
    vm->_alu.right_ = right;

    alu_compute(&vm->_alu);

    vm->_cpsr.N = vm->_alu.flags_.N;
    vm->_cpsr.Z = vm->_alu.flags_.Z;
    vm->_cpsr.V = vm->_alu.flags_.V;
    vm->_cpsr.C = vm->_alu.flags_.C;
    vm->_cpsr.S = vm->_alu.flags_.S;
}

static bool vm_validate_range(VirtualMachine *vm, size_t address, size_t size) {
    if (vm->_memorySize > SIZE_T_MAX - ZLVM_ROM_SIZE) {
        vm_set_state(vm, S_ERR_OUT_OF_MEMORY);
        return false;
    }

    size_t memory_end = ZLVM_ROM_SIZE + vm->_memorySize;
    if (size > memory_end || address > memory_end - size) {
        vm_set_state(vm, S_ERR_OUT_OF_MEMORY);
        return false;
    }
    return true;
}

static bool vm_effective_address(VirtualMachine *vm, word base, word offset, size_t size,
                                 size_t *address) {
    dword result = (dword)base + (dword)offset;
    if (result > SIZE_T_MAX || !vm_validate_range(vm, (size_t)result, size)) {
        vm_set_state(vm, S_ERR_OUT_OF_MEMORY);
        return false;
    }
    *address = (size_t)result;
    return true;
}

void vm_interrupt(VirtualMachine *vm, word code) {
    // TODO(emulator): replace host I/O interrupts with an injectable interface.
    switch (code) {
        case 0x01:
            fputc((byte)vm->_registers[R_A0].word_ + 0x60, stdout);
            break;
        case 0x02:
            fputc((byte)vm->_registers[R_A0].word_, stdout);
            break;
        case 0x10:
            vm->_registers[R_V0].word_ = (word)fgetc(stdin);
            break;
        case 0xFF:
            vm_set_state(vm, S_HALTED);
            break;
        default:
            break;
    }
}

void vm_syscall(VirtualMachine *vm) {
    // TODO(emulator): implement syscall services.
    switch (vm->_registers[R_SC].word_) {
        default:
            break;
    }
}
