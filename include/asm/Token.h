//
// Created by Stanislav on 2019-05-07.
//

#ifndef ZLVM_C_TOKEN_H
#define ZLVM_C_TOKEN_H

#include "Types.h"

typedef enum TokenType {
    TOK_COMMENT,
    TOK_NEWLINE,

    TOK_ID,
    TOK_REGISTER,
    TOK_LABEL_INIT,
    TOK_LABEL_USE,
    TOK_DIRECTIVE,

    TOK_COMMA,

    TOK_STRING_LITERAL,
    TOK_CHAR_LITERAL,

    TOK_INT_HEX,
    TOK_INT_DEC,
    TOK_INT_OCT,
    TOK_INT_BIN,
} TokenType;

typedef struct Token {
    TokenType type;
    size_t size;
    char* value;

    size_t pos;
    size_t line;
    size_t col;
} Token;

/**
 * Prints token representation to STDOUT
 */
void token_print(Token*);

/**
 * Free memory, used by token
 */
void token_free(Token*);

dword token_get_int_value(Token* t);

char token_get_char_value(char* t);

byte* token_get_string_value(char* val, size_t* __size);

byte* token_get_raw_data(Token* t, size_t* __size);

#endif //ZLVM_C_TOKEN_H
