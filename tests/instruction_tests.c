#include "VirtualMachineInternal.h"
#include "asm/zlasm.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static Instruction instruction(Opcode opcode, byte register1, byte register2, word immediate) {
    return (Instruction){
        .opcode_ = opcode,
        .condition_ = C_UNCONDITIONAL,
        .register1 = register1,
        .register2 = register2,
        .immediate = immediate,
    };
}

static void initialize_vm(VirtualMachine *vm, size_t ram_size) {
    *vm = (VirtualMachine){0};
    vm_initialize(vm, ram_size);
    vm->_registers[R_BP].word_ = ZLVM_ROM_SIZE;
    vm->_registers[R_SP].word_ = ZLVM_ROM_SIZE;
}

static void test_assembler_accepts_every_opcode(void) {
    char source[] = "nop\n"
                    "pop\n"
                    "popr $t0\n"
                    "pushr $t0\n"
                    "pushi 1\n"
                    "dup\n"
                    "movr $t0, $t1\n"
                    "movi $t0, 1\n"
                    "addr $t0, $t1\n"
                    "subr $t0, $t1\n"
                    "mulr $t0, $t1\n"
                    "divr $t0, $t1\n"
                    "modr $t0, $t1\n"
                    "addi $t0, 1\n"
                    "subi $t0, 1\n"
                    "muli $t0, 1\n"
                    "divi $t0, 1\n"
                    "modi $t0, 1\n"
                    "int 1\n"
                    "syscall\n"
                    "jmp 0\n"
                    "not $t0\n"
                    "andr $t0, $t1\n"
                    "orr $t0, $t1\n"
                    "xorr $t0, $t1\n"
                    "nandr $t0, $t1\n"
                    "norr $t0, $t1\n"
                    "andi $t0, 1\n"
                    "ori $t0, 1\n"
                    "xori $t0, 1\n"
                    "nandi $t0, 1\n"
                    "nori $t0, 1\n"
                    "inc $t0\n"
                    "dec $t0\n"
                    "loadb $t0, $t1, 0\n"
                    "storeb $t0, $t1, 0\n"
                    "loadh $t0, $t1, 0\n"
                    "storeh $t0, $t1, 0\n"
                    "loadw $t0, $t1, 0\n"
                    "storew $t0, $t1, 0\n"
                    "jmpal 0\n"
                    "ret\n"
                    "cmpr $t0, $t1\n"
                    "cmpi $t0, 1\n"
                    "cmpsr $t0, $t1\n"
                    "cmpsi $t0, 1\n"
                    "addsr $t0, $t1\n"
                    "subsr $t0, $t1\n"
                    "mulsr $t0, $t1\n"
                    "divsr $t0, $t1\n"
                    "modsr $t0, $t1\n"
                    "addsi $t0, 1\n"
                    "subsi $t0, 1\n"
                    "mulsi $t0, 1\n"
                    "divsi $t0, 1\n"
                    "modsi $t0, 1\n";

    size_t binary_size = 0;
    byte *binary = assemblySource(source, &binary_size);
    assert(binary_size == OPCODE_TOTAL * sizeof(Instruction));
    for (size_t index = 0; index < OPCODE_TOTAL; index++) {
        Instruction encoded = {0};
        memcpy(&encoded, binary + index * sizeof encoded, sizeof encoded);
        assert(encoded.opcode_ == (Opcode)index);
    }
    free(binary);

    char conditions[] = "nop uh\nnop ul\n";
    binary = assemblySource(conditions, &binary_size);
    Instruction encoded[2] = {0};
    memcpy(encoded, binary, sizeof encoded);
    assert(encoded[0].condition_ == C_UNSIGNED_HIGHER);
    assert(encoded[1].condition_ == C_UNSIGNED_LOWER_OR_SAME);
    free(binary);
}

static void test_every_opcode_dispatches(void) {
    for (Opcode opcode = NOP; opcode < OPCODE_TOTAL; opcode++) {
        VirtualMachine vm;
        initialize_vm(&vm, 512);
        vm._registers[R_T0].word_ = 8;
        vm._registers[R_T1].word_ = 2;

        if (opcode == POP || opcode == POPR || opcode == DUP) {
            assert(vm_push_word(&vm, 7));
        }

        word immediate = opcode >= LOADB && opcode <= STOREW ? 0 : 1;
        if (opcode == INT) {
            immediate = 0;
        }
        if (opcode >= LOADB && opcode <= STOREW) {
            vm._registers[R_T1].word_ = ZLVM_ROM_SIZE;
        }

        vm_run_instruction(&vm, instruction(opcode, R_T0, R_T1, immediate));
        assert(vm_get_state(&vm) == S_NORMAL);
        vm_destroy(&vm);
    }
}

static ALU calculate(Operation operation, word left, word right) {
    ALU alu = {.left_ = left, .right_ = right, .op_ = operation};
    alu_compute(&alu);
    return alu;
}

static void test_arithmetic_and_flags(void) {
    ALU alu = calculate(OP_DIV, 1, 0);
    assert((word)alu.result_ == 0);

    alu = calculate(OP_ADD, WORD_MAX, 1);
    assert((word)alu.result_ == 0 && alu.flags_.Z && alu.flags_.C);

    alu = calculate(OP_SUB, 3, 5);
    assert((word)alu.result_ == WORD_MAX - 1 && alu.flags_.N && !alu.flags_.C);
    alu = calculate(OP_SUB, 5, 3);
    assert((word)alu.result_ == 2 && alu.flags_.C);

    alu = calculate(OP_SDIV, (word)-1, 2);
    assert((word)alu.result_ == 0 && alu.flags_.S && alu.flags_.Z);
    alu = calculate(OP_SADD, (word)SWORD_MAX, 1);
    assert((word)alu.result_ == 0x80000000u && alu.flags_.S && alu.flags_.V && alu.flags_.N);
    alu = calculate(OP_SDIV, (word)SWORD_MIN, (word)-1);
    assert((word)alu.result_ == 0x80000000u && alu.flags_.V);
    alu = calculate(OP_SMOD, (word)SWORD_MIN, (word)-1);
    assert((word)alu.result_ == 0 && !alu.flags_.V);
}

static void test_instruction_effects(void) {
    VirtualMachine vm;
    initialize_vm(&vm, 512);

    vm_run_instruction(&vm, instruction(PUSHI, R_ZERO, R_ZERO, 0x12345678));
    vm_run_instruction(&vm, instruction(DUP, R_ZERO, R_ZERO, 0));
    vm_run_instruction(&vm, instruction(POPR, R_T0, R_ZERO, 0));
    vm_run_instruction(&vm, instruction(POPR, R_T1, R_ZERO, 0));
    assert(vm._registers[R_T0].word_ == 0x12345678);
    assert(vm._registers[R_T1].word_ == 0x12345678);

    vm._registers[R_T0].word_ = 0x11223344;
    vm._registers[R_T1].word_ = ZLVM_ROM_SIZE + 32;
    vm_run_instruction(&vm, instruction(STOREW, R_T0, R_T1, 0));
    vm._registers[R_T2].word_ = 0;
    vm_run_instruction(&vm, instruction(LOADW, R_T2, R_T1, 0));
    assert(vm._registers[R_T2].word_ == 0x11223344);

    vm._registers[R_PC].word_ = 8;
    vm_run_instruction(&vm, instruction(JMPAL, R_ZERO, R_ZERO, 100));
    assert(vm._registers[R_LP].word_ == 8 && vm._registers[R_PC].word_ == 100);
    vm_run_instruction(&vm, instruction(RET, R_ZERO, R_ZERO, 0));
    assert(vm._registers[R_PC].word_ == 8);
    vm_run_instruction(&vm, instruction(JMP, R_ZERO, R_ZERO, 44));
    assert(vm._registers[R_PC].word_ == 44);

    vm_destroy(&vm);
}

static void test_conditions(void) {
    VirtualMachine vm;
    initialize_vm(&vm, 512);

    assert(vm_check_condition(&vm, C_UNCONDITIONAL));
    vm._cpsr.Z = true;
    assert(vm_check_condition(&vm, C_ZERO_SET));
    assert(!vm_check_condition(&vm, C_ZERO_CLEAR));
    vm._cpsr.Z = false;
    vm._cpsr.N = true;
    assert(vm_check_condition(&vm, C_NEGATIVE_SET));
    assert(!vm_check_condition(&vm, C_NEGATIVE_CLEAR));
    vm._cpsr.V = true;
    assert(vm_check_condition(&vm, C_OVERFLOW_SET));
    assert(!vm_check_condition(&vm, C_OVERFLOW_CLEAR));
    vm._cpsr.C = true;
    assert(vm_check_condition(&vm, C_CARRY_SET));
    assert(!vm_check_condition(&vm, C_CARRY_CLEAR));
    assert(vm_check_condition(&vm, C_UNSIGNED_HIGHER));
    vm._cpsr.Z = true;
    assert(vm_check_condition(&vm, C_UNSIGNED_LOWER_OR_SAME));
    vm._cpsr.S = true;
    assert(vm_check_condition(&vm, C_SIGNED_SET));
    assert(!vm_check_condition(&vm, C_SIGNED_CLEAR));
    vm._cpsr.N = false;
    vm._cpsr.V = true;
    vm._cpsr.Z = false;
    assert(vm_check_condition(&vm, C_LESS));
    assert(vm_check_condition(&vm, C_LESS_OR_EQUALS));
    assert(!vm_check_condition(&vm, C_GREATER));
    assert(!vm_check_condition(&vm, C_GREATER_OR_EQUALS));
    vm_destroy(&vm);
}

static void test_vm_errors_are_atomic(void) {
    VirtualMachine vm;
    initialize_vm(&vm, 8);

    vm._registers[R_T0].word_ = 123;
    vm_run_instruction(&vm, instruction(DIVI, R_T0, R_ZERO, 0));
    assert(vm_get_state(&vm) == S_ERR_DIVISION_BY_ZERO);
    assert(vm._registers[R_T0].word_ == 123);
    vm_destroy(&vm);

    initialize_vm(&vm, 8);
    vm._registers[R_T0].word_ = 77;
    vm._registers[R_T1].word_ = ZLVM_ROM_SIZE + 6;
    vm_run_instruction(&vm, instruction(LOADW, R_T0, R_T1, 0));
    assert(vm_get_state(&vm) == S_ERR_OUT_OF_MEMORY);
    assert(vm._registers[R_T0].word_ == 77);
    vm_destroy(&vm);

    initialize_vm(&vm, 8);
    vm._registers[R_PC].word_ = ZLVM_ROM_SIZE + 7;
    vm_fetch_instruction(&vm);
    assert(vm_get_state(&vm) == S_ERR_OUT_OF_MEMORY);
    assert(vm._registers[R_PC].word_ == ZLVM_ROM_SIZE + 7);
    vm_destroy(&vm);

    initialize_vm(&vm, 8);
    vm._registers[R_T0].word_ = 0x11223344;
    vm._registers[R_T1].word_ = ZLVM_ROM_SIZE + 6;
    memset(vm._memory, 0xAA, vm._memorySize);
    vm_run_instruction(&vm, instruction(STOREW, R_T0, R_T1, 0));
    assert(vm_get_state(&vm) == S_ERR_OUT_OF_MEMORY);
    for (size_t index = 0; index < vm._memorySize; index++) {
        assert(vm._memory[index] == 0xAA);
    }
    vm_destroy(&vm);

    initialize_vm(&vm, 512);
    vm._registers[R_ZERO].word_ = 99;
    vm_run_instruction(&vm, instruction(MOVI, R_ZERO, R_ZERO, 42));
    assert(vm._registers[R_ZERO].word_ == 0);

    for (size_t count = 0; count < ZLVM_STACK_SIZE / ZLVM_WORD_SIZE; count++) {
        assert(vm_push_word(&vm, (word)count));
    }
    word full_stack_pointer = vm._registers[R_SP].word_;
    assert(!vm_push_word(&vm, 0));
    assert(vm_get_state(&vm) == S_ERR_STACK_OVERFLOW);
    assert(vm._registers[R_SP].word_ == full_stack_pointer);
    vm_destroy(&vm);

    initialize_vm(&vm, 512);
    vm._registers[R_T0].word_ = 77;
    vm_run_instruction(&vm, instruction(POPR, R_T0, R_ZERO, 0));
    assert(vm_get_state(&vm) == S_ERR_STACK_UNDERFLOW);
    assert(vm._registers[R_T0].word_ == 77);
    vm_destroy(&vm);
}

int main(void) {
    test_assembler_accepts_every_opcode();
    test_every_opcode_dispatches();
    test_arithmetic_and_flags();
    test_instruction_effects();
    test_conditions();
    test_vm_errors_are_atomic();
    return 0;
}
