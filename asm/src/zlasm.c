// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
//
// Created by Stanislav on 2019-06-04.
//

#include "zlasm.h"
#include <Lexer.h>
#include <Parser.h>
#include <Assembler.h>

byte* assemblySource(char* source, size_t* binarySize) {
    byte* res;

    LexerState* lexer = malloc(sizeof(LexerState));
    lexer_init(lexer, source);

    Token* tok = lexer_readToken(lexer);
    while (tok != NULL)
    {
        tok = lexer_readToken(lexer);
    }

    ParserContext* parser = malloc(sizeof(ParserContext));
    parser_init(parser);
    parser_parse(parser, tokenStream_new(lexer->_tokens));
    free(lexer);

    AssemblerContext* assembler = malloc(sizeof(AssemblerContext));
    asm_init(assembler);
    asm_processDirectives(assembler, parser);
    free(parser);
    asm_processLabels(assembler);
    res = asm_translate(assembler, binarySize);

    free(assembler);

    return res;
}