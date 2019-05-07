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
    char* ptr;
    struct TokenList* _tokens;
};

void lexer_init(struct LexerState*, char* source);

struct Token* lexer_read_token(struct LexerState*);

void lexer_clear(struct LexerState* state);

#endif //ZLVM_C_LEXER_H
