//
// Created by Stanislav on 2019-05-07.
//

#ifndef ZLVM_C_LEXER_H
#define ZLVM_C_LEXER_H

#include "Token.h"

struct TokenList {
    struct Token* value;
    struct TokenList* next;
};

struct LexerState {
    char* source;
    struct TokenList* _tokens;
    size_t pos;
    size_t line;
    size_t col;
};

void lexer_init(struct LexerState*, char* source);

char lexer_peek_char(struct LexerState*);

char lexer_next_char(struct LexerState*);

char* lexer_ahead(struct LexerState*);

struct Token* lexer_read_token(struct LexerState*);

void lexer_clear(struct LexerState* state);

#endif //ZLVM_C_LEXER_H
