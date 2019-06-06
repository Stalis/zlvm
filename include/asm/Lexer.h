//
// Created by Stanislav on 2019-05-07.
//

#ifndef ZLVM_C_LEXER_H
#define ZLVM_C_LEXER_H

#include "Token.h"

typedef struct TokenList {
    Token* value;
    struct TokenList* next;
} TokenList;

typedef struct TokenStream {
    TokenList* _first;
} TokenStream;

TokenStream* tokenStream_new(TokenList*);
Token* tokenStream_read(TokenStream* stream);
Token* tokenStream_peek(TokenStream* stream);
bool tokenStream_isEof(TokenStream* stream);


typedef struct LexerState {
    char* source;
    size_t _len;
    TokenList* _tokens;
    size_t pos;
    size_t line;
    size_t col;
} LexerState;

void lexer_init(LexerState*, char* source);

char lexer_peekChar(LexerState*);

char lexer_nextChar(LexerState*);

char* lexer_ahead(LexerState*);

Token* lexer_readToken(LexerState*);

void lexer_clear(LexerState* state);

#endif //ZLVM_C_LEXER_H
