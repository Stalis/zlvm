// Created by Stanislav on 2019-04-26.
//
#include "VirtualMachineInternal.h"

#include "Memory.h"

#include <stdio.h>
#include <string.h>

#if DEBUG

static inline void print_registers(VirtualMachine* vm, byte columns) {
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

void vm_initialize(VirtualMachine* vm, size_t ram_size) {
    for (size_t i = 0; i < ZLVM_REGISTER_COUNT; i++) {
        vm->_registers[i].word_ = 0;
    }
    for (size_t i = 0; i < ZLVM_ROM_SIZE; i++) {
        vm->_rom[i] = 0;
    }

    vm->_memorySize = ram_size;
    vm->_memory = vm_calloc(ram_size, sizeof *vm->_memory);

    vm->_cpsr.value_.word_ = 0;
    vm_set_state(vm, S_NORMAL);
}

void vm_destroy(VirtualMachine* vm) {
    if (vm == NULL) {
        return;
    }
    free(vm->_memory);
    vm->_memory = NULL;
    vm->_memorySize = 0;
}

void vm_loadDump(VirtualMachine* vm, const byte* program, size_t size) {
    if (size > ZLVM_ROM_SIZE) {
        vm_set_state(vm, S_ERR_OUT_OF_MEMORY);
        return;
    }
    for (size_t i = 0; i < size; i++) {
        vm->_rom[i] = program[i];
    }
}

State vm_run(VirtualMachine* vm) {
    vm->_registers[R_PC].word_ = 0; // set to start of rom
    vm->_registers[R_BP].word_ = ZLVM_ROM_SIZE;
    vm->_registers[R_SP].word_ = vm->_registers[R_BP].word_; // set to start of memory
    while (vm_has_no_error(vm) && !vm_has_state(vm, S_HALTED)) {
        vm_run_instruction(vm, vm_fetch_instruction(vm));
    }
#if DEBUG
    print_registers(vm, 4);
#endif
    return vm_get_state(vm);
}

byte vm_fetch_byte(VirtualMachine* vm) {
    return vm_read_byte(vm, vm->_registers[R_PC].word_++);
}

Instruction vm_fetch_instruction(VirtualMachine* vm) {
    byte bytes[sizeof(Instruction)] = {0};
    for (size_t index = 0; index < sizeof(Instruction); index++) {
        bytes[index] = vm_fetch_byte(vm);
    }

    Instruction instruction = {0};
    memcpy(&instruction, bytes, sizeof instruction);
    return instruction;
}

void vm_run_instruction(VirtualMachine* vm, Instruction instruction) {
    if (!vm_check_condition(vm, instruction.condition_)) {
        return;
    }

    if (instruction.register1 >= ZLVM_REGISTER_COUNT ||
        instruction.register2 >= ZLVM_REGISTER_COUNT) {
        vm_set_state(vm, S_ERR_INVALID_OPCODE);
        return;
    }

    Value* reg1 = &vm->_registers[instruction.register1];
    Value* reg2 = &vm->_registers[instruction.register2];
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

        case POPR:
            reg1->word_ = vm_pop_word(vm);
            break;
        case POP:
            vm_pop_word(vm);
            break;

        case PUSHR:
            vm_push_word(vm, reg1->word_);
            break;
        case PUSHI:
            vm_push_word(vm, imm);
            break;
        case DUP: {
            word value = vm_pop_word(vm);
            vm_push_word(vm, value);
            vm_push_word(vm, value);
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

        case LOADB:
            reg1->byte_ = vm_read_byte(vm, reg2->word_ + imm);
            break;
        case LOADH:
            reg1->hword_ = vm_read_hword(vm, reg2->word_ + imm);
            break;
        case LOADW:
            reg1->word_ = vm_read_word(vm, reg2->word_ + imm);
            break;

        case STOREB:
            vm_write_byte(vm, reg2->word_ + imm, reg1->byte_);
            break;
        case STOREH:
            vm_write_hword(vm, reg2->word_ + imm, reg1->hword_);
            break;
        case STOREW:
            vm_write_word(vm, reg2->word_ + imm, reg1->word_);
            break;

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

    if (write_alu_result) {
        reg1->word_ = vm->_alu.result_;
    }

    if (vm->_alu.op_ != OP_NOOP) {
        alu_reset(&vm->_alu);
    }
}

bool vm_check_condition(VirtualMachine* vm, Condition condition) {
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

byte vm_read_byte(VirtualMachine* vm, size_t address) {
    if (address >= ZLVM_ROM_SIZE) {
        address -= ZLVM_ROM_SIZE;

        if (address >= vm->_memorySize) {
            vm_set_state(vm, S_ERR_OUT_OF_MEMORY);
            return 0;
        }
        return vm->_memory[address];
    }
    return vm->_rom[address];
}

void vm_write_byte(VirtualMachine* vm, size_t address, byte value) {
    if (address >= ZLVM_ROM_SIZE) {
        address -= ZLVM_ROM_SIZE;
        if (address >= vm->_memorySize) {
            vm_set_state(vm, S_ERR_OUT_OF_MEMORY);
            return;
        }
        vm->_memory[address] = value;
        return;
    }
    vm->_rom[address] = value;
}

hword vm_read_hword(VirtualMachine* vm, size_t address) {
    hword result = 0;
    byte* bytes = (byte*)&result;
    for (size_t index = 0; index < sizeof result; index++) {
        bytes[index] = vm_read_byte(vm, address + index);
    }
    return result;
}

void vm_write_hword(VirtualMachine* vm, size_t address, hword value) {
    const byte* bytes = (const byte*)&value;
    for (size_t index = 0; index < sizeof value; index++) {
        vm_write_byte(vm, address + index, bytes[index]);
    }
}

word vm_read_word(VirtualMachine* vm, size_t address) {
    word result = 0;
    byte* bytes = (byte*)&result;
    for (size_t index = 0; index < sizeof result; index++) {
        bytes[index] = vm_read_byte(vm, address + index);
    }
    return result;
}

void vm_write_word(VirtualMachine* vm, size_t address, word value) {
    const byte* bytes = (const byte*)&value;
    for (size_t index = 0; index < sizeof value; index++) {
        vm_write_byte(vm, address + index, bytes[index]);
    }
}

dword vm_read_dword(VirtualMachine* vm, size_t address) {
    dword result = 0;
    byte* bytes = (byte*)&result;
    for (size_t index = 0; index < sizeof result; index++) {
        bytes[index] = vm_read_byte(vm, address + index);
    }
    return result;
}

void vm_write_dword(VirtualMachine* vm, size_t address, dword value) {
    const byte* bytes = (const byte*)&value;
    for (size_t index = 0; index < sizeof value; index++) {
        vm_write_byte(vm, address + index, bytes[index]);
    }
}

word vm_pop_word(VirtualMachine* vm) {
    if (vm->_registers[R_SP].word_ <= vm->_registers[R_BP].word_) {
        vm_set_state(vm, S_ERR_STACK_UNDERFLOW);
        return 0;
    }
    word result = vm_read_word(vm, vm->_registers[R_SP].word_);
    vm->_registers[R_SP].word_ -= ZLVM_WORD_SIZE;
    return result;
}

void vm_push_word(VirtualMachine* vm, word value) {
    word used_stack = vm->_registers[R_SP].word_ - vm->_registers[R_BP].word_;
    if (used_stack > ZLVM_STACK_SIZE - ZLVM_WORD_SIZE) {
        vm_set_state(vm, S_ERR_STACK_OVERFLOW);
        return;
    }
    vm->_registers[R_SP].word_ += ZLVM_WORD_SIZE;
    vm_write_word(vm, vm->_registers[R_SP].word_, value);
}

bool vm_has_no_error(VirtualMachine* vm) {
    return !is_error(vm->_cpsr.ST);
}

bool vm_has_state(VirtualMachine* vm, State state) {
    return vm->_cpsr.ST == state;
}

void vm_set_state(VirtualMachine* vm, State state) {
    if (vm_has_no_error(vm)) {
        vm->_cpsr.ST = state;
    }
}

enum State vm_get_state(VirtualMachine* vm) {
    return vm->_cpsr.ST;
}

void vm_do_operation(VirtualMachine* vm, Operation op, word left, word right) {
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

void vm_interrupt(VirtualMachine* vm, word code) {
    // TODO(emulator): replace host I/O interrupts with an injectable interface.
    switch (code) {
        case 0x01:
            fputc(vm->_registers[R_A0].byte_ + 0x60, stdout);
            break;
        case 0x02:
            fputc(vm->_registers[R_A0].byte_, stdout);
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

void vm_syscall(VirtualMachine* vm) {
    // TODO(emulator): implement syscall services.
    switch (vm->_registers[R_SC].word_) {
        default:
            break;
    }
}
