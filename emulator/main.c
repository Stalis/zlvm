#include "VirtualMachine.h"
#include "asm/zlasm.h"
#include "src/Memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static byte *read_source(const char *path, size_t *binary_size);
static byte *read_binary(const char *path, size_t *binary_size);
static void print_state(State state);

int main(int argc, char **argv) {
    const size_t memory_size = 4096;

    printf("Encoded instruction size: %d bytes\n", ZLVM_INSTRUCTION_SIZE);
    printf("Operations count: %d\n", OPCODE_TOTAL);
    printf("Size of machine word: %d bytes\n", ZLVM_WORD_SIZE);
    printf("ROM size: %d bytes\n", ZLVM_ROM_SIZE);
    printf("RAM size: %zu bytes\n", memory_size);
    printf("Stack size: %d bytes\n", ZLVM_STACK_SIZE);
    printf("==========================================\n");

    bool is_binary_mode = argc == 3 && strcmp(argv[1], "--binary") == 0;
    bool is_source_mode = argc == 2 && strcmp(argv[1], "--binary") != 0;
    if (!is_source_mode && !is_binary_mode) {
        fprintf(stderr, "Usage: %s <assembly-file>\n       %s --binary <binary-file>\n", argv[0],
                argv[0]);
        return EXIT_FAILURE;
    }

    size_t binary_size = 0;
    byte *binary =
        is_binary_mode ? read_binary(argv[2], &binary_size) : read_source(argv[1], &binary_size);

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

    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Unable to open assembly source: %s\n", path);
        exit(EXIT_FAILURE);
    }
    char *source = vm_malloc(capacity + 1);

    while (true) {
        size_t available = capacity - length;
        size_t bytes_read = fread(source + length, 1, available, file);
        length += bytes_read;

        if (bytes_read < available) {
            break;
        }

        capacity += growth_size;
        source = vm_realloc(source, capacity + 1);
    }

    bool read_failed = ferror(file);
    bool close_failed = fclose(file) != 0;
    if (read_failed) {
        fprintf(stderr, "Unable to read assembly source: %s\n", path);
    }
    if (close_failed) {
        fprintf(stderr, "Unable to close assembly source: %s\n", path);
    }
    if (read_failed || close_failed) {
        free(source);
        exit(EXIT_FAILURE);
    }

    source[length] = '\0';
    byte *binary = assemblySource(source, binary_size);
    free(source);
    return binary;
}

static byte *read_binary(const char *path, size_t *binary_size) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Unable to open binary image: %s\n", path);
        exit(EXIT_FAILURE);
    }

    byte *binary = vm_malloc(ZLVM_ROM_SIZE + 1);
    size_t image_size = fread(binary, 1, ZLVM_ROM_SIZE + 1, file);
    bool read_failed = ferror(file);
    bool close_failed = fclose(file) != 0;

    if (read_failed) {
        fprintf(stderr, "Unable to read binary image: %s\n", path);
    }
    if (close_failed) {
        fprintf(stderr, "Unable to close binary image: %s\n", path);
    }
    if (read_failed || close_failed) {
        free(binary);
        exit(EXIT_FAILURE);
    }
    if (image_size == 0) {
        fprintf(stderr, "Binary image is empty: %s\n", path);
        free(binary);
        exit(EXIT_FAILURE);
    }
    if (image_size > ZLVM_ROM_SIZE) {
        fprintf(stderr, "Binary image exceeds ROM size: %s\n", path);
        free(binary);
        exit(EXIT_FAILURE);
    }

    *binary_size = image_size;
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
        case S_ERR_DIVISION_BY_ZERO:
            printf("Error: Division by zero");
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
