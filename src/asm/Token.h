//
// Created by Stanislav on 2019-05-07.
//

#ifndef ZLVM_C_TOKEN_H
#define ZLVM_C_TOKEN_H

#include <assert.h>
#include <memory.h>
#include "../common/Types.h"
#include "../common/Error.h"

enum TokenType {
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
};

struct Token {
    enum TokenType type;
    size_t size;
    char* value;

    size_t pos;
    size_t line;
    size_t col;
};

/**
 * Prints token representation to STDOUT
 */
void token_print(struct Token*);

/**
 * Free memory, used by token
 */
void token_free(struct Token*);

static inline dword token_get_int_value(struct Token* t) {
    switch (t->type) {
        case TOK_INT_HEX:
            return strtoull(t->value, NULL, 16);
        case TOK_INT_DEC:
            return strtoull(t->value, NULL, 10);
        case TOK_INT_OCT:
            return strtoull(t->value, NULL, 8);
        case TOK_INT_BIN:
            return strtoull(t->value, NULL, 2);
        default:
            CRASH("Not int token");
    }
    return 0;
}

static inline char token_get_char_value(char* t) {
    if (t[0] == '\\') {
        switch (t[1]) {
            case 'n':
                return '\n';
            case 'r':
                return '\r';
            case 't':
                return '\t';
            case 'a':
                return '\a';
            case 'f':
                return '\f';
            case 'v':
                return '\v';
            case 'b':
                return '\b';
            case '\\':
                return '\\';
            default:
                CRASH("Invalid escape character");
        }
    }
    return t[0];
}

static byte* token_get_string_value(char* val, size_t* __size) {
    byte* res = calloc(strlen(val), sizeof(char));
    for (*__size = 0; val[*__size] != '\0'; *__size += 1) {
        res[*__size] = (byte)val[*__size];
    }
    res = realloc(res, *__size);
    return res;
}

static byte* token_get_raw_data(struct Token* t, size_t* __size) {
    byte* res = NULL;
    switch (t->type) {
        case TOK_STRING_LITERAL:
            res = token_get_string_value(t->value, __size);
            break;
        case TOK_CHAR_LITERAL:
            *__size = 1;
            res = malloc((*__size) * sizeof(byte));
            *res = (byte)token_get_char_value(t->value);
            break;
        case TOK_INT_HEX:
        case TOK_INT_DEC:
        case TOK_INT_OCT:
        case TOK_INT_BIN:
            res = calloc(1, sizeof(dword));
            *__size = sizeof(dword);
            break;
        default:
            CRASH("Is not data token");
            break;
    }
    return res;
}
#endif //ZLVM_C_TOKEN_H
