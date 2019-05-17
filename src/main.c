#include <stdio.h>
#include "emulator/VirtualMachine.h"
#include "asm/asm.h"
#include "asm/Lexer.h"

const char* test_file = "/Users/stalis/Develop/Projects/zlvm/zlvm-c/test.bin";
const char* test_source = "/Users/stalis/Develop/Projects/zlvm/zlvm-c/test.asm";

byte* readFile(const char* path, size_t* size);
byte* readSource(const char* path, size_t* size);
void printState(enum State state);

void test_parser(const char* path);

int main() {
    printf("Size of Instruction: %lu bytes\n", sizeof(struct Instruction));
    printf("Size of machine word: %lu bytes\n", __ZLVM_WORD_SIZE);
    printf("Operations count: %d\n", OPCODE_TOTAL);
    printf("\n");

    test_parser(test_source);

/*
    byte* buffer;
    size_t size;
    //size = readFile(test_file, &buffer);
    buffer = readSource(test_source, &size);

    struct VirtualMachine vm = {};
    initialize(&vm);
    loadDump(&vm, buffer, size);
    free(buffer);
    enum State state = run(&vm);

    printf("Result code: %d\n", state);
    printState(state);

    return state;
*/
}

byte* readFile(const char* path, size_t* __size) {
    size_t size = 0;
    byte* buffer = (byte*) malloc(__ZLVM_MEMORY_SIZE);
    FILE* file = fopen(path, "rb");

    for (size = 0; (size < __ZLVM_MEMORY_SIZE) && (!feof(file)); size++) {
        buffer[size] = (byte) fgetc(file);
    }

    fclose(file);

    *__size = size;
    return buffer;
}

byte* readSource(const char* path, size_t* size) {
    const size_t step_size = 1024;
    size_t cur_size = step_size;
    char* source = (char*) malloc(sizeof(char) * cur_size);
    size_t i = 0;

    FILE* file = fopen(path, "r");
    while (!feof(file)) {
        source[i] = (char) fgetc(file);
        i++;

        if (i >= cur_size) {
            cur_size += step_size;
            source = realloc(source, sizeof(char) * cur_size);
        }
    }
    fclose(file);

    return assembly(source, size);
}

void printState(enum State state) {
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

void test_parser(const char* path) {
    const size_t step_size = 1024;
    size_t cur_size = step_size;
    char* source = (char*) malloc(sizeof(char) * cur_size);
    size_t i = 0;

    FILE* file = fopen(path, "r");
    while (!feof(file)) {
        source[i] = (char) fgetc(file);
        i++;

        if (i >= cur_size) {
            cur_size += step_size;
            source = realloc(source, sizeof(char) * cur_size);
        }
    }
    fclose(file);

    struct LexerState lexer;
    lexer_init(&lexer, source);

    struct Token* tok = lexer_read_token(&lexer);
    while (tok != NULL) {
        token_print(tok);
        tok = lexer_read_token(&lexer);
    }

    lexer_clear(&lexer);
}