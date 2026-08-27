// clang-format off
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "VirtualMachineInternal.h"
#include "asm/zlasm.h"
// clang-format on

static const char *const register_names[R_TOTAL] = {
    "R_ZERO", "R_AT", "R_V0", "R_V1", "R_V2", "R_V3", "R_A0", "R_A1", "R_A2", "R_A3", "R_T0",
    "R_T1",   "R_T2", "R_T3", "R_T4", "R_T5", "R_T6", "R_T7", "R_S0", "R_S1", "R_S2", "R_S3",
    "R_S4",   "R_S5", "R_S6", "R_S7", "R_K0", "R_SC", "R_LP", "R_SP", "R_BP", "R_PC",
};

static const word expected_registers[R_TOTAL] = {
    [R_V0] = 0x000000FF, [R_T0] = 0x0000012C, [R_T2] = 0x00001024, [R_LP] = 0x00000058,
    [R_SP] = 0x00001004, [R_BP] = 0x00001000, [R_PC] = 0x00000118,
};

static const char *state_name(State state) {
    switch (state) {
        case S_HALTED:
            return "S_HALTED";
        case S_NORMAL:
            return "S_NORMAL";
        case S_ERR_BEGIN:
            return "S_ERR_BEGIN";
        case S_ERR_OUT_OF_MEMORY:
            return "S_ERR_OUT_OF_MEMORY";
        case S_ERR_STACK_OVERFLOW:
            return "S_ERR_STACK_OVERFLOW";
        case S_ERR_STACK_UNDERFLOW:
            return "S_ERR_STACK_UNDERFLOW";
        case S_ERR_DIVISION_BY_ZERO:
            return "S_ERR_DIVISION_BY_ZERO";
        case S_ERR_INVALID_CONDITION:
            return "S_ERR_INVALID_CONDITION";
        case S_ERR_INVALID_OPCODE:
            return "S_ERR_INVALID_OPCODE";
        case S_ERR_END:
            return "S_ERR_END";
        case S_TOTAL:
            return "S_TOTAL";
    }
    return "unknown";
}

static byte *read_file(const char *path, size_t *file_size, bool terminate) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Unable to open fixture: %s\n", path);
        return NULL;
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        fprintf(stderr, "Unable to seek fixture: %s\n", path);
        if (fclose(file) != 0) {
            fprintf(stderr, "Unable to close fixture: %s\n", path);
        }
        return NULL;
    }

    long length = ftell(file);
    if (length < 0 || fseek(file, 0, SEEK_SET) != 0) {
        fprintf(stderr, "Unable to determine fixture size: %s\n", path);
        if (fclose(file) != 0) {
            fprintf(stderr, "Unable to close fixture: %s\n", path);
        }
        return NULL;
    }

    size_t size = (size_t)length;
    byte *contents = malloc(size + (terminate ? 1 : 0));
    if (contents == NULL) {
        fprintf(stderr, "Unable to allocate %zu bytes for fixture: %s\n", size, path);
        if (fclose(file) != 0) {
            fprintf(stderr, "Unable to close fixture: %s\n", path);
        }
        return NULL;
    }
    bool read_failed = fread(contents, 1, size, file) != size;
    bool close_failed = fclose(file) != 0;
    if (read_failed || close_failed) {
        fprintf(stderr, "Unable to read fixture: %s\n", path);
        free(contents);
        return NULL;
    }
    if (terminate) {
        contents[size] = '\0';
    }
    *file_size = size;
    return contents;
}

static size_t compare_states(VirtualMachine *source_vm, VirtualMachine *binary_vm) {
    size_t failures = 0;
    State source_state = vm_get_state(source_vm);
    State binary_state = vm_get_state(binary_vm);

    if (source_state != binary_state) {
        fprintf(stderr, "State differs: source=%s (%d), binary=%s (%d)\n", state_name(source_state),
                source_state, state_name(binary_state), binary_state);
        failures++;
    }
    if (source_state != S_HALTED || binary_state != S_HALTED) {
        fprintf(stderr, "Expected S_HALTED: source=%s (%d), binary=%s (%d)\n",
                state_name(source_state), source_state, state_name(binary_state), binary_state);
        failures++;
    }

    for (size_t index = 0; index < R_TOTAL; index++) {
        word source_value = source_vm->_registers[index].word_;
        word binary_value = binary_vm->_registers[index].word_;
        word expected_value = expected_registers[index];
        if (source_value != binary_value) {
            fprintf(stderr,
                    "Register %s[%zu] differs: source=0x%08" PRIX32 ", binary=0x%08" PRIX32 "\n",
                    register_names[index], index, source_value, binary_value);
            failures++;
        }
        if (source_value != expected_value || binary_value != expected_value) {
            fprintf(stderr,
                    "Register %s[%zu] expected 0x%08" PRIX32 ": source=0x%08" PRIX32
                    ", binary=0x%08" PRIX32 "\n",
                    register_names[index], index, expected_value, source_value, binary_value);
            failures++;
        }
    }

    size_t ram_differences = 0;
    for (size_t offset = 0; offset < 4096; offset++) {
        if (source_vm->_memory[offset] != binary_vm->_memory[offset]) {
            if (ram_differences == 0) {
                fprintf(stderr, "RAM differs at offset %zu: source=0x%02X, binary=0x%02X\n", offset,
                        (unsigned)source_vm->_memory[offset], (unsigned)binary_vm->_memory[offset]);
            }
            ram_differences++;
        }
    }
    if (ram_differences != 0) {
        fprintf(stderr, "RAM has %zu differing byte(s)\n", ram_differences);
        failures++;
    }

    const size_t addresses[] = {4096, 5192};
    for (size_t index = 0; index < sizeof addresses / sizeof addresses[0]; index++) {
        word source_value = vm_read_word(source_vm, addresses[index]);
        word binary_value = vm_read_word(binary_vm, addresses[index]);
        if (source_value != 0x00001024 || binary_value != 0x00001024) {
            fprintf(stderr,
                    "Word at logical address %zu expected 0x00001024: source=0x%08" PRIX32
                    ", binary=0x%08" PRIX32 "\n",
                    addresses[index], source_value, binary_value);
            failures++;
        }
    }
    return failures;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source-file> <binary-file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    size_t ignored_size = 0;
    byte *source = read_file(argv[1], &ignored_size, true);
    size_t file_binary_size = 0;
    byte *file_binary = read_file(argv[2], &file_binary_size, false);
    if (source == NULL || file_binary == NULL) {
        free(source);
        free(file_binary);
        return EXIT_FAILURE;
    }

    size_t source_binary_size = 0;
    byte *source_binary = assemblySource((char *)source, &source_binary_size);
    free(source);

    VirtualMachine source_vm = {0};
    VirtualMachine binary_vm = {0};
    vm_initialize(&source_vm, 4096);
    vm_initialize(&binary_vm, 4096);
    vm_loadDump(&source_vm, source_binary, source_binary_size);
    vm_loadDump(&binary_vm, file_binary, file_binary_size);
    free(source_binary);
    free(file_binary);

    vm_run(&source_vm);
    vm_run(&binary_vm);
    size_t failures = compare_states(&source_vm, &binary_vm);

    vm_destroy(&source_vm);
    vm_destroy(&binary_vm);
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
