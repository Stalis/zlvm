//
// Created by Stanislav on 2019-05-07.
//

#ifndef ZLVM_C_LEXER_H
#define ZLVM_C_LEXER_H

#include "Token.h"

typedef struct TokenList {
    struct Token* value;
    struct TokenList* next;
} token_list_t;

typedef struct TokenStream {
    token_list_t* _first;
} token_stream_t;

struct TokenStream* tokenStream_new(struct TokenList*);
struct Token* tokenStream_read(token_stream_t* stream);
struct Token* tokenStream_peek(struct TokenStream* stream);
bool tokenStream_isEof(token_stream_t* stream);


struct LexerState {
    char* source;
    size_t _len;
    struct TokenList* _tokens;
    size_t pos;
    size_t line;
    size_t col;
};

void lexer_init(struct LexerState*, char* source);

char lexer_peekChar(struct LexerState*);

char lexer_nextChar(struct LexerState*);

char* lexer_ahead(struct LexerState*);

struct Token* lexer_readToken(struct LexerState*);

void lexer_clear(struct LexerState* state);

#endif //ZLVM_C_LEXER_H
