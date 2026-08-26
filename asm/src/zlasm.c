//
// Created by Stanislav on 2019-06-04.
//

#include "zlasm.h"

#include "Memory.h"

#include <Assembler.h>
#include <Lexer.h>
#include <Parser.h>

byte *assemblySource(char *source, size_t *binary_size) {
    byte *result;

    LexerState *lexer = asm_malloc(sizeof(LexerState));
    lexer_init(lexer, source);

    Token *token = lexer_readToken(lexer);
    while (token != NULL) {
        token = lexer_readToken(lexer);
    }

    ParserContext *parser = asm_malloc(sizeof(ParserContext));
    parser_init(parser);
    parser_parse(parser, tokenStream_new(lexer->_tokens));
    asm_free(lexer);

    AssemblerContext *assembler = asm_malloc(sizeof(AssemblerContext));
    asm_init(assembler);
    asm_processDirectives(assembler, parser);
    asm_free(parser);
    asm_processLabels(assembler);
    result = asm_translate(assembler, binary_size);

    asm_free(assembler);

    return result;
}
