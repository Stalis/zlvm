// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Created by Stanislav on 2019-05-27.
//
#include <stdio.h>
#include <assert.h>
#include "VirtualMachine.h"
#include "asm/zlasm.h"
#include "asm/Lexer.h"
#include "asm/Parser.h"
#include "asm/Assembler.h"
#include "src/Memory.h"

static void test_parser(const char* path);
static byte* readSource(const char* path, size_t* size);
static char* stripExtension(char* path);
static void writeBinary(const char* path, byte* data, size_t size);

int main(int argc, char** argv) {
    if (argc <= 1) {
        printf("Too few arguments");
        exit(-1);
    }

    char* file = argv[1];

#ifdef DEBUG
    test_parser(test_source);
#endif

    size_t size = 0;
    byte* bin = readSource(file, &size);

    stripExtension(file);
    strcat(file, ".bin");

    writeBinary(file, bin, size);

    return 0;
}


static byte* readSource(const char* path, size_t* size) {
    const size_t step_size = 1024;
    size_t cur_size = step_size;
    char* source = malloc(sizeof(char) * cur_size);

    size_t i = 0;

    FILE* file = fopen(path, "r");
    if (file == NULL) {
        printf("File not found: %s", path);
        exit(-1);
    }

    while (!feof(file)) {
        source[i] = (char) fgetc(file);
        i++;

        if (i >= cur_size) {
            cur_size += step_size;
            source = realloc(source, sizeof(char) * cur_size);
        }
    }
    fclose(file);

    return assemblySource(source, size);
}

static char* stripExtension(char* path) {
    char* end = path + strlen(path);
    while (end > path && *end != '.') {
        --end;
    }

    if (end > path) {
        *end = '\0';
    }

    return path;
}

static void writeBinary(const char* path, byte* data, size_t size) {
    FILE* file = fopen(path, "wb");
    for (size_t i = 0; i < size; i++) {
        fputc(data[i], file);
    }
    fclose(file);
}

#ifdef DEBUG
static void test_parser(const char* path) {
    const size_t step_size = 1024;
    size_t cur_size = step_size;
    char* source = (char*) malloc(sizeof(char) * cur_size);
    size_t i = 0;

    FILE* file = fopen(path, "r");
    while (!feof(file))
    {
        source[i] = (char) fgetc(file);
        i++;

        if (i >= cur_size)
        {
            cur_size += step_size;
            source = realloc(source, sizeof(char) * cur_size);
        }
    }
    fclose(file);

    struct LexerState lexer;
    lexer_init(&lexer, source);

    Token* tok = lexer_readToken(&lexer);
    while (tok != NULL)
    {
        token_print(tok);
        tok = lexer_readToken(&lexer);
    }
    free(source);

    printf("======================================\n");

    struct ParserContext parser;
    parser_init(&parser);

    parser_parse(&parser, tokenStream_new(lexer._tokens));
    lexer_clear(&lexer);

    struct LineList* ptr = parser.lines;
    while (NULL != ptr)
    {
        line_print(ptr->value);
        ptr = ptr->next;
    }

    printf("======================================\n");

    struct AssemblerContext asm_;
    asm_init(&asm_);
    asm_processDirectives(&asm_, &parser);
    parser_clear(&parser);

    ptr = asm_.lines;
    while (NULL != ptr)
    {
        line_print(ptr->value);
        ptr = ptr->next;
    }

    asm_processLabels(&asm_);

    size_t size = 0;
    byte* res = asm_translate(&asm_, &size);

    VirtualMachine vm;
    vm_initialize(&vm, 1024);
    vm_loadDump(&vm, res, size);
    State state = vm_run(&vm);
    exit(state);
}

#endif