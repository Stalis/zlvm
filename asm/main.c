//
// Created by Stanislav on 2019-05-27.
//
#include "include/Lexer.h"
#include "include/Parser.h"

const char* test_source = "/Users/stalis/Develop/Projects/zlvm/zlvm-c/test.asm";

static void test_parser(const char* path);

int main(int argc, char** argv) {
    test_parser(test_source);
    return 0;
}

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

    struct Token* tok = lexer_read_token(&lexer);
    while (tok != NULL)
    {
        token_print(tok);
        tok = lexer_read_token(&lexer);

    }

    struct ParserContext parser;
    context_init(&parser);

    context_read_tokens(&parser, lexer._tokens);
    context_proc_directives(&parser);

    context_clear(&parser);
    lexer_clear(&lexer);
}