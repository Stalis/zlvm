//
// Created by Stanislav on 2019-04-26.
//

#include "VirtualMachine.h"

void initialize(struct VirtualMachine* this) {
    for (size_t i = 0; i < __ZLVM_REGISTER_COUNT; i++) {
        this->_registers[i].dword_ = 0;
    }
    for (size_t i = 0; i < __ZLVM_MEMORY_SIZE + __ZLVM_STACK_SIZE; i++) {
        this->_memory[i] = 0;
    }

    this->_pc.dword_ = 0;
    this->_sp.dword_ = 0;
    this->_bp.dword_ = 0;
    this->_sc.dword_ = 0;
    this->_cpsr.value_.dword_ = 0;
    setState(this, S_NORMAL);
}

void loadDump(struct VirtualMachine* vm, byte* program, size_t size) {
    if (size > __ZLVM_MEMORY_SIZE) {
        setState(vm, S_ERR_OUT_OF_MEMORY);
        return;
    }
    for (size_t i = 0; i < size; i++) {
        vm->_memory[__ZLVM_STACK_SIZE + i] = program[i];
    }
}

enum State run(struct VirtualMachine* vm) {
    vm->_pc.dword_ = __ZLVM_STACK_SIZE; // set to end of stack
    while (notError(vm) && !checkState(vm, S_HALTED)) {
        runInstruction(vm, fetchInstruction(vm));
    }
    return getState(vm);
}

byte fetchByte(struct VirtualMachine* vm) {
    return readByte(vm, vm->_pc.dword_++);
}

struct Instruction fetchInstruction(struct VirtualMachine* vm) {
    byte* buf = (byte*) malloc(sizeof(struct Instruction));
    byte bufArr[sizeof(struct Instruction)] = {0};
    for (size_t i = 0; i < sizeof(struct Instruction); i++) {
        bufArr[i] = fetchByte(vm);
    }
    return *((struct Instruction*) &bufArr);
}

void runInstruction(struct VirtualMachine* vm, struct Instruction instruction) {
    if (!checkCondition(vm, instruction.condition_))
        return;

    union Value* reg1 = &vm->_registers[instruction.register1];
    union Value* reg2 = &vm->_registers[instruction.register2];
    dword imm = instruction.immediate;

    switch (instruction.opcode_) {
        case NOP:
            break;

        case POPR:
            reg1->dword_ = popDword(vm);
            break;
        case POP:
            popDword(vm);
            break;

        case PUSHR:
            pushDword(vm, reg1->dword_);
            break;
        case PUSHI:
            pushDword(vm, imm);
            break;
        case DUP: {
            dword buf = popDword(vm);
            pushDword(vm, buf);
            pushDword(vm, buf);
        }
            break;

        case MOVR:
            reg1->dword_ = reg2->dword_;
            break;
        case MOVI:
            reg1->dword_ = imm;
            break;

        case ADDR:
            doOperation(vm, OP_ADD, reg1->dword_, reg2->dword_);
            goto __alu_write_result;
        case SUBR:
            doOperation(vm, OP_SUB, reg1->dword_, reg2->dword_);
            goto __alu_write_result;
        case MULR:
            doOperation(vm, OP_MUL, reg1->dword_, reg2->dword_);
            goto __alu_write_result;
        case DIVR:
            doOperation(vm, OP_DIV, reg1->dword_, reg2->dword_);
            goto __alu_write_result;
        case MODR:
            doOperation(vm, OP_MOD, reg1->dword_, reg2->dword_);
            goto __alu_write_result;

        case ADDI:
            doOperation(vm, OP_ADD, reg1->dword_, imm);
            goto __alu_write_result;
        case SUBI:
            doOperation(vm, OP_SUB, reg1->dword_, imm);
            goto __alu_write_result;
        case MULI:
            doOperation(vm, OP_MUL, reg1->dword_, imm);
            goto __alu_write_result;
        case DIVI:
            doOperation(vm, OP_DIV, reg1->dword_, imm);
            goto __alu_write_result;
        case MODI:
            doOperation(vm, OP_MOD, reg1->dword_, imm);
            goto __alu_write_result;

        case NOT:
            doOperation(vm, OP_NOT, reg1->dword_, 0);
            goto __alu_write_result;
        case ANDR:
            doOperation(vm, OP_AND, reg1->dword_, reg2->dword_);
            goto __alu_write_result;
        case ORR:
            doOperation(vm, OP_OR, reg1->dword_, reg2->dword_);
            goto __alu_write_result;
        case XORR:
            doOperation(vm, OP_XOR, reg1->dword_, reg2->dword_);
            goto __alu_write_result;
        case NANDR:
            doOperation(vm, OP_NAND, reg1->dword_, reg2->dword_);
            goto __alu_write_result;
        case NORR:
            doOperation(vm, OP_NOR, reg1->dword_, reg2->dword_);
            goto __alu_write_result;

        case ANDI:
            doOperation(vm, OP_AND, reg1->dword_, imm);
            goto __alu_write_result;
        case ORI:
            doOperation(vm, OP_OR, reg1->dword_, imm);
            goto __alu_write_result;
        case XORI:
            doOperation(vm, OP_XOR, reg1->dword_, imm);
            goto __alu_write_result;
        case NANDI:
            doOperation(vm, OP_NAND, reg1->dword_, imm);
            goto __alu_write_result;
        case NORI:
            doOperation(vm, OP_NOR, reg1->dword_, imm);
            goto __alu_write_result;

        case INC:
            doOperation(vm, OP_INC, reg1->dword_, 0);
            goto __alu_write_result;
        case DEC:
            doOperation(vm, OP_DEC, reg1->dword_, 0);
            goto __alu_write_result;

        case LOADR:
            reg1->dword_ = readDword(vm, reg2->dword_);
            break;
        case LOADI:
            reg1->dword_ = readDword(vm, imm);
            break;
        case STORER:
            writeDword(vm, reg1->dword_, reg2->dword_);
            break;
        case STOREI:
            writeDword(vm, reg1->dword_, imm);
            break;

        case INT:
            interrupt(vm, imm);
            break;
        case SYSCALL:
            syscall(vm);
            break;

        case JMPAL:
            vm->_lp.dword_ = vm->_pc.dword_;
        case JMP:
            vm->_pc.dword_ = imm;
            break;
        case RET:
            vm->_pc.dword_ = vm->_lp.dword_;
            break;

        case CMPI:
            doOperation(vm, OP_SUB, reg1->dword_, imm);
            break;
        case CMPR:
            doOperation(vm, OP_SUB, reg1->dword_, reg2->dword_);
            break;

        case HALT:
            setState(vm, S_HALTED);
            break;

        default:
            setState(vm, S_ERR_INVALID_OPCODE);
            break;

        __alu_write_result:
            reg1->dword_ = vm->_alu.result_;
            break;
    }

    if (vm->_alu.op_ != OP_NOOP) {
        alu_reset(&vm->_alu);
    }
}

bool checkCondition(struct VirtualMachine* vm, enum Condition condition) {
    switch (condition) {
        case C_UNCONDITIONAL:
            return true;
        case C_EQUALS:
            return (vm->_cpsr.Z);
        case C_NOT_EQUALS:
            return !(vm->_cpsr.Z);
        case C_LESS:
            return (vm->_cpsr.N) ^ (vm->_cpsr.V);
        case C_LESS_OR_EQUALS:
            return (vm->_cpsr.Z) || ((vm->_cpsr.N) ^ (vm->_cpsr.V));
        case C_GREATER:
            return !((vm->_cpsr.Z) || ((vm->_cpsr.N) ^ (vm->_cpsr.V)));
        case C_GREATER_OR_EQUALS:
            return !((vm->_cpsr.N) ^ (vm->_cpsr.V));
        default:
            setState(vm, S_ERR_INVALID_CONDITION);
            return false;
    }
}

byte readByte(struct VirtualMachine* this, size_t address) {
    if (address > __ZLVM_MEMORY_SIZE + __ZLVM_STACK_SIZE) {
        setState(this, S_ERR_OUT_OF_MEMORY);
        return 0;
    }
    return this->_memory[address];
}

void writeByte(struct VirtualMachine* this, size_t address, byte value) {
    if (address > __ZLVM_MEMORY_SIZE + __ZLVM_STACK_SIZE) {
        setState(this, S_ERR_OUT_OF_MEMORY);
        return;
    }
    this->_memory[address] = value;
}

word readWord(struct VirtualMachine* this, size_t address) {
    word res = 0;
    byte* ptr = (byte*) &res;
    for (size_t i = 0; i < sizeof(word); i++) {
        ptr[i] = readByte(this, address + i);
    }
    return res;
}

void writeWord(struct VirtualMachine* this, size_t address, word value) {
    byte* buf = (byte*) &value;
    for (size_t i = 0; i < sizeof(word); i++) {
        writeByte(this, address + i, buf[i]);
    }
}

dword readDword(struct VirtualMachine* this, size_t address) {
    dword res = 0;
    byte* ptr = (byte*) &res;
    for (size_t i = 0; i < sizeof(dword); i++) {
        ptr[i] = readByte(this, address + i);
    }
    return res;
}

void writeDword(struct VirtualMachine* this, size_t address, dword value) {
    byte* buf = (byte*) &value;
    for (size_t i = 0; i < sizeof(dword); i++) {
        writeByte(this, address + i, buf[i]);
    }
}

qword readQword(struct VirtualMachine* this, size_t address) {
    qword res = 0;
    byte* ptr = (byte*) &res;
    for (size_t i = 0; i < sizeof(qword); i++) {
        ptr[i] = readByte(this, address + i);
    }
    return res;
}

void writeQword(struct VirtualMachine* this, size_t address, qword value) {
    byte* buf = (byte*) &value;
    for (size_t i = 0; i < sizeof(qword); i++) {
        writeByte(this, address + i, buf[i]);
    }
}

dword popDword(struct VirtualMachine* vm) {
    if ((sdword) (vm->_sp.dword_ - __ZLVM_MACHINE_WORD) < 0) {
        setState(vm, S_ERR_STACK_UNDERFLOW);
        return 0;
    }
    dword res = readDword(vm, vm->_sp.dword_);
    vm->_sp.dword_ -= __ZLVM_MACHINE_WORD;
    return res;
}

void pushDword(struct VirtualMachine* vm, dword value) {
    if (vm->_sp.dword_ + __ZLVM_MACHINE_WORD >= __ZLVM_STACK_SIZE) {
        setState(vm, S_ERR_STACK_OVERFLOW);
        return;
    }
    vm->_sp.dword_ += __ZLVM_MACHINE_WORD;
    writeDword(vm, vm->_sp.dword_, value);
}

bool notError(struct VirtualMachine* vm) {
    return !is_error(vm->_cpsr.ST);
}

bool checkState(struct VirtualMachine* vm, enum State state) {
    return vm->_cpsr.ST == state;
}

void setState(struct VirtualMachine* vm, enum State state) {
    if (notError(vm)) {
        vm->_cpsr.ST = state;
    }
}

enum State getState(struct VirtualMachine* vm) {
    return vm->_cpsr.ST;
}

void doOperation(struct VirtualMachine* vm, enum Operation op, dword left, dword right) {
    vm->_alu.op_ = op;
    vm->_alu.left_ = left;
    vm->_alu.right_ = right;

    alu_compute(&vm->_alu);

    vm->_cpsr.N = vm->_alu.flags_.N;
    vm->_cpsr.Z = vm->_alu.flags_.Z;
    vm->_cpsr.V = vm->_alu.flags_.V;
    vm->_cpsr.C = vm->_alu.flags_.C;
}

void interrupt(struct VirtualMachine* vm, dword code) {
    // TODO: прерывания :)))
}

void syscall(struct VirtualMachine* vm) {
    // TODO: софтовые прерывания :)))
}