//
// Created by Stanislav on 2019-06-04.
//

#include <Lexer.h>
#include <Parser.h>
#include <Assembler.h>
#include "zlasm.h"

byte* assembly(char* source, size_t* __size) {
    byte* res;

    LexerState* lexer = malloc(sizeof(LexerState));
    lexer_init(lexer, source);

    Token* tok = lexer_readToken(lexer);
    while (tok != NULL)
    {
        token_print(tok);
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
    res = asm_translate(assembler, __size);

    free(assembler);

    return res;
}