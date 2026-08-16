#include "VirtualMachine.h"
#include "asm/zlasm.h"
#include "src/Memory.h"

#include <stdio.h>
#include <stdlib.h>

static byte *read_source(const char *path, size_t *binary_size);
static void print_state(State state);

int main(int argc, char **argv) {
    const size_t memory_size = 4096;

    printf("Size of Instruction: %zu bytes\n", sizeof(Instruction));
    printf("Operations count: %d\n", OPCODE_TOTAL);
    printf("Size of machine word: %d bytes\n", ZLVM_WORD_SIZE);
    printf("ROM size: %d bytes\n", ZLVM_ROM_SIZE);
    printf("RAM size: %zu bytes\n", memory_size);
    printf("Stack size: %d bytes\n", ZLVM_STACK_SIZE);
    printf("==========================================\n");

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <assembly-file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    size_t binary_size = 0;
    byte *binary = read_source(argv[1], &binary_size);

    VirtualMachine vm = {0};
    vm_initialize(&vm, memory_size);
    vm_loadDump(&vm, binary, binary_size);
    free(binary);

    State state = vm_run(&vm);
    vm_destroy(&vm);

    printf("==========================================\n");
    printf("Result code: %d\n", state);
    print_state(state);

    return state;
}

static byte *read_source(const char *path, size_t *binary_size) {
    const size_t growth_size = 1024;
    size_t capacity = growth_size;
    size_t length = 0;
    char *source = vm_malloc(capacity + 1);

    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Unable to open assembly source: %s\n", path);
        exit(EXIT_FAILURE);
    }

    while (true) {
        size_t available = capacity - length;
        size_t bytes_read = fread(source + length, 1, available, file);
        length += bytes_read;

        if (bytes_read < available) {
            if (ferror(file)) {
                fprintf(stderr, "Unable to read assembly source: %s\n", path);
                fclose(file);
                free(source);
                exit(EXIT_FAILURE);
            }
            break;
        }

        capacity += growth_size;
        source = vm_realloc(source, capacity + 1);
    }

    if (fclose(file) != 0) {
        fprintf(stderr, "Unable to close assembly source: %s\n", path);
        free(source);
        exit(EXIT_FAILURE);
    }

    source[length] = '\0';
    byte *binary = assemblySource(source, binary_size);
    free(source);
    return binary;
}

static void print_state(State state) {
    switch (state) {
        case S_NORMAL:
            printf("Normal");
            break;
        case S_ERR_OUT_OF_MEMORY:
            printf("Error: Out of memory");
            break;
        case S_ERR_STACK_OVERFLOW:
            printf("Error: Stack overflow");
            break;
        case S_ERR_STACK_UNDERFLOW:
            printf("Error: Stack underflow");
            break;
        case S_ERR_INVALID_CONDITION:
            printf("Error: Invalid condition");
            break;
        case S_ERR_INVALID_OPCODE:
            printf("Error: Invalid opcode");
            break;
        case S_HALTED:
            printf("Halted");
            break;
        default:
            printf("Unknown error");
            break;
    }
    printf("\n");
}
