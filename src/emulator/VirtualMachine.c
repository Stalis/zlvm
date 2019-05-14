//
// Created by Stanislav on 2019-04-26.
//
#include "VirtualMachine.h"

#if DEBUG

#include <printf.h>

static inline void printRegisters(struct VirtualMachine* vm, byte columns) {
    for (word i = 0; i < __ZLVM_REGISTER_COUNT; i += columns)
    {
        for (byte j = 0; j < columns && j < __ZLVM_REGISTER_COUNT; j++)
        {
            printf("[r%d]:\t%d\t", i + j, vm->_registers[i + j].word_);
        }
        printf("\n");
    }

    printf("[PC]:\t%d\t[SP]:\t%d\t[BP]:\t%d\t[SC]:\t%d\t[LP]:\t%d\n",
           vm->_pc.word_, vm->_sp.word_, vm->_bp.word_, vm->_sc.word_, vm->_lp.word_);
    printf("[CPSR]: N:%d Z:%d V:%d C:%d S:%d ST:%d\n",
           vm->_cpsr.N, vm->_cpsr.Z, vm->_cpsr.V, vm->_cpsr.C, vm->_cpsr.S, vm->_cpsr.ST);
}

#endif

void initialize(struct VirtualMachine* this) {
    for (size_t i = 0; i < __ZLVM_REGISTER_COUNT; i++)
    {
        this->_registers[i].word_ = 0;
    }
    for (size_t i = 0; i < __ZLVM_MEMORY_SIZE + __ZLVM_STACK_SIZE; i++)
    {
        this->_memory[i] = 0;
    }

    this->_pc.word_ = 0;
    this->_sp.word_ = 0;
    this->_bp.word_ = 0;
    this->_sc.word_ = 0;
    this->_cpsr.value_.word_ = 0;
    setState(this, S_NORMAL);
}

void loadDump(struct VirtualMachine* vm, byte* program, size_t size) {
    if (size > __ZLVM_MEMORY_SIZE)
    {
        setState(vm, S_ERR_OUT_OF_MEMORY);
        return;
    }
    for (size_t i = 0; i < size; i++)
    {
        vm->_memory[__ZLVM_STACK_SIZE + i] = program[i];
    }
}

enum State run(struct VirtualMachine* vm) {
    vm->_pc.word_ = __ZLVM_STACK_SIZE; // set to end of stack
    while (notError(vm) && !checkState(vm, S_HALTED))
    {
        runInstruction(vm, fetchInstruction(vm));
    }
#if DEBUG
    printRegisters(vm, 4);
#endif
    return getState(vm);
}

byte fetchByte(struct VirtualMachine* vm) {
    return readByte(vm, vm->_pc.word_++);
}

struct Instruction fetchInstruction(struct VirtualMachine* vm) {
    byte bufArr[sizeof(struct Instruction)] = {0};
    for (size_t i = 0; i < sizeof(struct Instruction); i++)
    {
        bufArr[i] = fetchByte(vm);
    }
    return *((struct Instruction*) &bufArr);
}

void runInstruction(struct VirtualMachine* vm, struct Instruction instruction) {
    if (!checkCondition(vm, instruction.condition_))
        return;

    union Value* reg1 = &vm->_registers[instruction.register1];
    union Value* reg2 = &vm->_registers[instruction.register2];
    word imm = instruction.immediate;

    switch (instruction.opcode_)
    {
        case NOP:
            break;

        case POPR:
            reg1->word_ = popWord(vm);
            break;
        case POP:
            popWord(vm);
            break;

        case PUSHR:
            pushWord(vm, reg1->word_);
            break;
        case PUSHI:
            pushWord(vm, imm);
            break;
        case DUP:
        {
            word buf = popWord(vm);
            pushWord(vm, buf);
            pushWord(vm, buf);
        }
            break;

        case MOVR:
            reg1->word_ = reg2->word_;
            break;
        case MOVI:
            reg1->word_ = imm;
            break;

        case ADDR:
            doOperation(vm, OP_ADD, reg1->word_, reg2->word_);
            goto __alu_write_result;
        case SUBR:
            doOperation(vm, OP_SUB, reg1->word_, reg2->word_);
            goto __alu_write_result;
        case MULR:
            doOperation(vm, OP_MUL, reg1->word_, reg2->word_);
            goto __alu_write_result;
        case DIVR:
            doOperation(vm, OP_DIV, reg1->word_, reg2->word_);
            goto __alu_write_result;
        case MODR:
            doOperation(vm, OP_MOD, reg1->word_, reg2->word_);
            goto __alu_write_result;

        case ADDI:
            doOperation(vm, OP_ADD, reg1->word_, imm);
            goto __alu_write_result;
        case SUBI:
            doOperation(vm, OP_SUB, reg1->word_, imm);
            goto __alu_write_result;
        case MULI:
            doOperation(vm, OP_MUL, reg1->word_, imm);
            goto __alu_write_result;
        case DIVI:
            doOperation(vm, OP_DIV, reg1->word_, imm);
            goto __alu_write_result;
        case MODI:
            doOperation(vm, OP_MOD, reg1->word_, imm);
            goto __alu_write_result;

        case ADDSR:
            doOperation(vm, OP_SADD, reg1->word_, reg2->word_);
            goto __alu_write_result;
        case SUBSR:
            doOperation(vm, OP_SSUB, reg1->word_, reg2->word_);
            goto __alu_write_result;
        case MULSR:
            doOperation(vm, OP_SMUL, reg1->word_, reg2->word_);
            goto __alu_write_result;
        case DIVSR:
            doOperation(vm, OP_SDIV, reg1->word_, reg2->word_);
            goto __alu_write_result;
        case MODSR:
            doOperation(vm, OP_SMOD, reg1->word_, reg2->word_);
            goto __alu_write_result;

        case ADDSI:
            doOperation(vm, OP_SADD, reg1->word_, imm);
            goto __alu_write_result;
        case SUBSI:
            doOperation(vm, OP_SSUB, reg1->word_, imm);
            goto __alu_write_result;
        case MULSI:
            doOperation(vm, OP_SMUL, reg1->word_, imm);
            goto __alu_write_result;
        case DIVSI:
            doOperation(vm, OP_SDIV, reg1->word_, imm);
            goto __alu_write_result;
        case MODSI:
            doOperation(vm, OP_SMOD, reg1->word_, imm);
            goto __alu_write_result;

        case NOT:
            doOperation(vm, OP_NOT, reg1->word_, 0);
            goto __alu_write_result;
        case ANDR:
            doOperation(vm, OP_AND, reg1->word_, reg2->word_);
            goto __alu_write_result;
        case ORR:
            doOperation(vm, OP_OR, reg1->word_, reg2->word_);
            goto __alu_write_result;
        case XORR:
            doOperation(vm, OP_XOR, reg1->word_, reg2->word_);
            goto __alu_write_result;
        case NANDR:
            doOperation(vm, OP_NAND, reg1->word_, reg2->word_);
            goto __alu_write_result;
        case NORR:
            doOperation(vm, OP_NOR, reg1->word_, reg2->word_);
            goto __alu_write_result;

        case ANDI:
            doOperation(vm, OP_AND, reg1->word_, imm);
            goto __alu_write_result;
        case ORI:
            doOperation(vm, OP_OR, reg1->word_, imm);
            goto __alu_write_result;
        case XORI:
            doOperation(vm, OP_XOR, reg1->word_, imm);
            goto __alu_write_result;
        case NANDI:
            doOperation(vm, OP_NAND, reg1->word_, imm);
            goto __alu_write_result;
        case NORI:
            doOperation(vm, OP_NOR, reg1->word_, imm);
            goto __alu_write_result;

        case INC:
            doOperation(vm, OP_INC, reg1->word_, 0);
            goto __alu_write_result;
        case DEC:
            doOperation(vm, OP_DEC, reg1->word_, 0);
            goto __alu_write_result;

        case LOADB:
            reg1->word_ = readByte(vm, reg2->word_ + imm);
            break;
        case LOADH:
            reg1->word_ = readHword(vm, reg2->word_ + imm);
            break;
        case LOADW:
            reg1->word_ = readWord(vm, reg2->word_ + imm);
            break;

        case STOREB:
            writeByte(vm, reg2->word_ + imm, reg1->byte_);
            break;
        case STOREH:
            writeHword(vm, reg2->word_ + imm, reg1->hword_);
            break;
        case STOREW:
            writeWord(vm, reg2->word_ + imm, reg1->word_);
            break;

        case INT:
            interrupt(vm, imm);
            break;
        case SYSCALL:
            syscall(vm);
            break;

        case JMPAL:
            vm->_lp.word_ = vm->_pc.word_;
        case JMP:
            vm->_pc.word_ = imm;
            break;
        case RET:
            vm->_pc.word_ = vm->_lp.word_;
            break;

        case CMPI:
            doOperation(vm, OP_SUB, reg1->word_, imm);
            break;
        case CMPR:
            doOperation(vm, OP_SUB, reg1->word_, reg2->word_);
            break;
        case CMPSI:
            doOperation(vm, OP_SSUB, reg1->word_, imm);
            break;
        case CMPSR:
            doOperation(vm, OP_SUB, reg1->word_, reg2->word_);
            break;

        default:
            setState(vm, S_ERR_INVALID_OPCODE);
            break;

        __alu_write_result:
            reg1->word_ = vm->_alu.result_;
            break;
    }

    if (vm->_alu.op_ != OP_NOOP)
    {
        alu_reset(&vm->_alu);
    }
}

bool checkCondition(struct VirtualMachine* vm, enum Condition condition) {
    switch (condition)
    {
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
            setState(vm, S_ERR_INVALID_CONDITION);
            return false;
    }
}

byte readByte(struct VirtualMachine* this, size_t address) {
    if (address > __ZLVM_MEMORY_SIZE + __ZLVM_STACK_SIZE)
    {
        setState(this, S_ERR_OUT_OF_MEMORY);
        return 0;
    }
    return this->_memory[address];
}

void writeByte(struct VirtualMachine* this, size_t address, byte value) {
    if (address > __ZLVM_MEMORY_SIZE + __ZLVM_STACK_SIZE)
    {
        setState(this, S_ERR_OUT_OF_MEMORY);
        return;
    }
    this->_memory[address] = value;
}

hword readHword(struct VirtualMachine* this, size_t address) {
    hword res = 0;
    byte* ptr = (byte*) &res;
    for (size_t i = 0; i < sizeof(hword); i++)
    {
        ptr[i] = readByte(this, address + i);
    }
    return res;
}

void writeHword(struct VirtualMachine* this, size_t address, hword value) {
    byte* buf = (byte*) &value;
    for (size_t i = 0; i < sizeof(hword); i++)
    {
        writeByte(this, address + i, buf[i]);
    }
}

word readWord(struct VirtualMachine* this, size_t address) {
    word res = 0;
    byte* ptr = (byte*) &res;
    for (size_t i = 0; i < sizeof(word); i++)
    {
        ptr[i] = readByte(this, address + i);
    }
    return res;
}

void writeWord(struct VirtualMachine* this, size_t address, word value) {
    byte* buf = (byte*) &value;
    for (size_t i = 0; i < sizeof(word); i++)
    {
        writeByte(this, address + i, buf[i]);
    }
}

dword readDword(struct VirtualMachine* this, size_t address) {
    dword res = 0;
    byte* ptr = (byte*) &res;
    for (size_t i = 0; i < sizeof(dword); i++)
    {
        ptr[i] = readByte(this, address + i);
    }
    return res;
}

void writeDword(struct VirtualMachine* this, size_t address, dword value) {
    byte* buf = (byte*) &value;
    for (size_t i = 0; i < sizeof(dword); i++)
    {
        writeByte(this, address + i, buf[i]);
    }
}

word popWord(struct VirtualMachine* this) {
    if ((sword) (this->_sp.word_ - __ZLVM_WORD_SIZE) < 0)
    {
        setState(this, S_ERR_STACK_UNDERFLOW);
        return 0;
    }
    word res = readWord(this, this->_sp.word_);
    this->_sp.word_ -= __ZLVM_WORD_SIZE;
    return res;
}

void pushWord(struct VirtualMachine* this, word value) {
    if (this->_sp.word_ + __ZLVM_WORD_SIZE >= __ZLVM_STACK_SIZE)
    {
        setState(this, S_ERR_STACK_OVERFLOW);
        return;
    }
    this->_sp.word_ += __ZLVM_WORD_SIZE;
    writeWord(this, this->_sp.word_, value);
}

bool notError(struct VirtualMachine* vm) {
    return !is_error(vm->_cpsr.ST);
}

bool checkState(struct VirtualMachine* vm, enum State state) {
    return vm->_cpsr.ST == state;
}

void setState(struct VirtualMachine* vm, enum State state) {
    if (notError(vm))
    {
        vm->_cpsr.ST = state;
    }
}

enum State getState(struct VirtualMachine* vm) {
    return vm->_cpsr.ST;
}

void doOperation(struct VirtualMachine* vm, enum Operation op, word left, word right) {
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

void interrupt(struct VirtualMachine* vm, word code) {
    // TODO: прерывания :)))
    switch (code)
    {
        case 0x10:
            printf("%c", vm->_memory[vm->_registers[0].word_]);
            break;
        case 0xFF:
            setState(vm, S_HALTED);
            break;
        default:
            break;
    }
}

void syscall(struct VirtualMachine* vm) {
    // TODO: софтовые прерывания :)))
}