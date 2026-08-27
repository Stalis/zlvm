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

static Instruction decode_instruction(const byte *input) {
    Instruction result;
    assert(instruction_decode(&result, input, ZLVM_INSTRUCTION_SIZE));
    return result;
}

static void initialize_vm(VirtualMachine *vm, size_t ram_size) {
    *vm = (VirtualMachine){0};
    vm_initialize(vm, ram_size);
    vm->_registers[R_BP].word_ = ZLVM_ROM_SIZE;
    vm->_registers[R_SP].word_ = ZLVM_ROM_SIZE;
}

static void test_instruction_codec(void) {
    const byte expected[ZLVM_INSTRUCTION_SIZE] = {0x07, 0x01, 0x0A, 0x0B, 0x78, 0x56, 0x34, 0x12};
    Instruction value = {
        .opcode_ = MOVI,
        .condition_ = C_ZERO_SET,
        .register1 = R_T0,
        .register2 = R_T1,
        .immediate = 0x12345678,
    };
    byte encoded[ZLVM_INSTRUCTION_SIZE];
    assert(instruction_encode(encoded, sizeof encoded, &value));
    assert(memcmp(encoded, expected, sizeof expected) == 0);

    Instruction decoded = decode_instruction(expected);
    assert(decoded.opcode_ == MOVI);
    assert(decoded.condition_ == C_ZERO_SET);
    assert(decoded.register1 == R_T0);
    assert(decoded.register2 == R_T1);
    assert(decoded.immediate == 0x12345678);
}

static void test_instruction_codec_validation(void) {
    Instruction value = instruction(MOVI, R_T0, R_T1, 0x12345678);
    byte output[ZLVM_INSTRUCTION_SIZE];
    byte expected[ZLVM_INSTRUCTION_SIZE];
    memset(output, 0xAA, sizeof output);
    memcpy(expected, output, sizeof expected);

    assert(!instruction_encode(NULL, sizeof output, &value));
    assert(!instruction_encode(output, ZLVM_INSTRUCTION_SIZE - 1, &value));
    assert(memcmp(output, expected, sizeof output) == 0);
    value.opcode_ = OPCODE_TOTAL;
    assert(!instruction_encode(output, sizeof output, &value));
    assert(memcmp(output, expected, sizeof output) == 0);
    value.opcode_ = MOVI;
    value.condition_ = C_TOTAL;
    assert(!instruction_encode(output, sizeof output, &value));
    assert(memcmp(output, expected, sizeof output) == 0);
    assert(!instruction_encode(output, sizeof output, NULL));

    Instruction decoded = instruction(NOP, R_ZERO, R_ZERO, 0xAAAAAAAA);
    assert(!instruction_decode(NULL, output, sizeof output));
    assert(!instruction_decode(&decoded, NULL, sizeof output));
    assert(!instruction_decode(&decoded, output, ZLVM_INSTRUCTION_SIZE - 1));
    assert(decoded.opcode_ == NOP);
    assert(decoded.immediate == 0xAAAAAAAA);
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
    assert(binary_size == OPCODE_TOTAL * ZLVM_INSTRUCTION_SIZE);
    for (size_t index = 0; index < OPCODE_TOTAL; index++) {
        Instruction encoded = decode_instruction(binary + index * ZLVM_INSTRUCTION_SIZE);
        assert(encoded.opcode_ == (Opcode)index);
    }
    free(binary);

    char conditions[] = "nop uh\nnop ul\n";
    binary = assemblySource(conditions, &binary_size);
    assert(decode_instruction(binary).condition_ == C_UNSIGNED_HIGHER);
    assert(decode_instruction(binary + ZLVM_INSTRUCTION_SIZE).condition_ ==
           C_UNSIGNED_LOWER_OR_SAME);
    free(binary);
}

static void test_assembler_layout(void) {
    char source[] = "jmp #target\n"
                    ".byte 0xff\n"
                    "target:\n"
                    "movi zs $t0, 0x12345678\n";
    size_t binary_size = 0;
    byte *binary = assemblySource(source, &binary_size);
    assert(binary_size == ZLVM_INSTRUCTION_SIZE * 2 + 1);
    assert(decode_instruction(binary).immediate == ZLVM_INSTRUCTION_SIZE + 1);
    assert(binary[ZLVM_INSTRUCTION_SIZE] == 0xFF);

    const byte expected[] = {0x07, 0x01, 0x0A, 0x00, 0x78, 0x56, 0x34, 0x12};
    assert(memcmp(binary + ZLVM_INSTRUCTION_SIZE + 1, expected, sizeof expected) == 0);
    free(binary);
}

static void test_directive_encoding(void) {
    char source[] = ".byte 0x12\n"
                    ".hword 0x1234\n"
                    ".word 0x12345678\n"
                    ".dword 0x0123456789abcdef\n"
                    ".ascii 0x1234, 'A', \"BC\"\n"
                    ".asciiz \"X\"\n"
                    ".space 2\n";
    const byte expected[] = {0x12, 0x34, 0x12, 0x78, 0x56, 0x34, 0x12, 0xEF, 0xCD, 0xAB, 0x89, 0x67,
                             0x45, 0x23, 0x01, 0x34, 0x41, 0x42, 0x43, 0x58, 0x00, 0x00, 0x00};
    size_t binary_size = 0;
    byte *binary = assemblySource(source, &binary_size);
    assert(binary_size == sizeof expected);
    assert(memcmp(binary, expected, sizeof expected) == 0);
    free(binary);
}

static void test_instruction_fetch(void) {
    const byte image[] = {0xFF, 0x07, 0x01, 0x0A, 0x0B, 0x78, 0x56, 0x34, 0x12};
    VirtualMachine vm;
    initialize_vm(&vm, 8);
    vm_loadDump(&vm, image, sizeof image);
    vm._registers[R_PC].word_ = 1;

    Instruction fetched = vm_fetch_instruction(&vm);
    assert(fetched.opcode_ == MOVI);
    assert(fetched.condition_ == C_ZERO_SET);
    assert(fetched.register1 == R_T0);
    assert(fetched.register2 == R_T1);
    assert(fetched.immediate == 0x12345678);
    assert(vm._registers[R_PC].word_ == 1 + ZLVM_INSTRUCTION_SIZE);
    vm_destroy(&vm);
}

static void test_decoded_instruction_validation(void) {
    const byte invalid[][ZLVM_INSTRUCTION_SIZE] = {
        {0xFF, C_UNCONDITIONAL, R_ZERO, R_ZERO, 0, 0, 0, 0},
        {NOP, 0xFF, R_ZERO, R_ZERO, 0, 0, 0, 0},
        {NOP, C_UNCONDITIONAL, R_TOTAL, R_ZERO, 0, 0, 0, 0},
    };
    const State expected[] = {
        S_ERR_INVALID_OPCODE,
        S_ERR_INVALID_CONDITION,
        S_ERR_INVALID_OPCODE,
    };

    for (size_t index = 0; index < sizeof invalid / sizeof invalid[0]; index++) {
        VirtualMachine vm;
        initialize_vm(&vm, 8);
        vm_run_instruction(&vm, decode_instruction(invalid[index]));
        assert(vm_get_state(&vm) == expected[index]);
        vm_destroy(&vm);
    }
}

static void test_little_endian_memory(void) {
    VirtualMachine vm;
    initialize_vm(&vm, 32);
    size_t address = ZLVM_ROM_SIZE + 1;

    vm_write_dword(&vm, address, 0x0123456789ABCDEF);
    const byte expected[] = {0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01};
    assert(memcmp(vm._memory + 1, expected, sizeof expected) == 0);
    assert(vm_read_hword(&vm, address) == 0xCDEF);
    assert(vm_read_word(&vm, address) == 0x89ABCDEF);
    assert(vm_read_dword(&vm, address) == 0x0123456789ABCDEF);

    vm._registers[R_T0].word_ = 0xAABBCCDD;
    vm._registers[R_T1].word_ = (word)address + 8;
    vm_run_instruction(&vm, instruction(STOREB, R_T0, R_T1, 0));
    vm_run_instruction(&vm, instruction(STOREH, R_T0, R_T1, 1));
    assert(vm._memory[9] == 0xDD);
    assert(vm._memory[10] == 0xDD && vm._memory[11] == 0xCC);

    vm._registers[R_T2].word_ = 0xFFFF0000;
    vm_run_instruction(&vm, instruction(LOADH, R_T2, R_T1, 1));
    assert(vm._registers[R_T2].word_ == 0xFFFFCCDD);
    vm_destroy(&vm);
}

static void test_directive_vm_integration(void) {
    char source[] = ".hword 0x1234\n.word 0x89abcdef\n";
    size_t binary_size = 0;
    byte *binary = assemblySource(source, &binary_size);
    VirtualMachine vm;
    initialize_vm(&vm, 8);
    vm_loadDump(&vm, binary, binary_size);
    free(binary);

    vm._registers[R_T0].word_ = 0;
    vm_run_instruction(&vm, instruction(LOADH, R_T1, R_T0, 0));
    vm_run_instruction(&vm, instruction(LOADW, R_T2, R_T0, 2));
    assert(vm._registers[R_T1].word_ == 0x1234);
    assert(vm._registers[R_T2].word_ == 0x89ABCDEF);
    vm_destroy(&vm);
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
    test_instruction_codec();
    test_instruction_codec_validation();
    test_assembler_accepts_every_opcode();
    test_assembler_layout();
    test_directive_encoding();
    test_instruction_fetch();
    test_decoded_instruction_validation();
    test_little_endian_memory();
    test_directive_vm_integration();
    test_every_opcode_dispatches();
    test_arithmetic_and_flags();
    test_instruction_effects();
    test_conditions();
    test_vm_errors_are_atomic();
    return 0;
}
