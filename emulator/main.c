// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include <stdio.h>
#include "VirtualMachine.h"
#include "../asm/include/zlasm.h"
#include "src/Error.h"
#include "src/Memory.h"

static const char* test_file = "/Users/stalis/Develop/Projects/zlvm/zlvm-c/test.bin";
static const char* test_source = "/Users/stalis/Develop/Projects/zlvm/zlvm-c/test.asm";

static byte* readFile(const char* path, size_t* size);
static byte* readSource(const char* path, size_t* size);
static void printState(enum State state);

int main() {
    printf("Size of Instruction: %lu bytes\n", sizeof(struct Instruction));
    printf("Size of machine word: %lu bytes\n", __ZLVM_WORD_SIZE);
    printf("Operations count: %d\n", OPCODE_TOTAL);
    printf("\n");

    byte* buffer;
    size_t size;
    //size = readFile(test_file, &buffer);
    buffer = readSource(test_source, &size);

    struct VirtualMachine vm = {};
    vm_initialize(&vm, 1024);
    vm_loadDump(&vm, buffer, size);
    free(buffer);
    enum State state = vm_run(&vm);

    printf("Result code: %d\n", state);
    printState(state);

    return state;
}

static byte* readFile(const char* path, size_t* __size) {
    size_t size = 0;
    byte* buffer = malloc_s(__ZLVM_ROM_SIZE);

    FILE* file = fopen(path, "rb");

    for (size = 0; (size < __ZLVM_ROM_SIZE) && (!feof(file)); size++)
    {
        buffer[size] = (byte) fgetc(file);
    }

    fclose(file);

    *__size = size;
    return buffer;
}

static byte* readSource(const char* path, size_t* size) {
    const size_t step_size = 1024;
    size_t cur_size = step_size;
    char* source = malloc_s(sizeof(char) * cur_size);

    size_t i = 0;

    FILE* file = fopen(path, "r");
    while (!feof(file)) {
        source[i] = (char) fgetc(file);
        i++;

        if (i >= cur_size) {
            cur_size += step_size;
            source = realloc_s(source, sizeof(char) * cur_size);
        }
    }
    fclose(file);

    return assembly(source, size);
}

static void printState(enum State state) {
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
    }
    printf("\n");
}