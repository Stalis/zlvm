// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
// Created by Stanislav on 2019-05-07.
//

#include <memory.h>
#include <assert.h>

#include "Token.h"
#include "Memory.h"

static inline const char* get_token_type_name(enum TokenType);

void token_print(struct Token* t) {
    if (t->type == TOK_NEWLINE)
    {
        fprintf(stdout, "(%4lu:%3lu:%2lu) %12s[%2lu]\n", t->pos, t->line, t->col, get_token_type_name(t->type),
                t->size);
    }
    else
    {
        fprintf(stdout, "(%4lu:%3lu:%2lu) %12s[%2lu] %s\n", t->pos, t->line, t->col, get_token_type_name(t->type),
                t->size, t->value);
    }
}

void token_free(struct Token* t) {
    free(t->value);
    free(t);
}

static inline const char* get_token_type_name(enum TokenType t) {
    switch (t)
    {
        case TOK_COMMENT:
            return "COMMENT";
        case TOK_NEWLINE:
            return "NEWLINE";
        case TOK_ID:
            return "ID";
        case TOK_REGISTER:
            return "REGISTER";
        case TOK_LABEL_INIT:
            return "LABEL_INIT";
        case TOK_LABEL_USE:
            return "LABEL_USE";
        case TOK_DIRECTIVE:
            return "DIRECTIVE";
        case TOK_STRING_LITERAL:
            return "STRING_LIT";
        case TOK_CHAR_LITERAL:
            return "CHAR_LIT";
        case TOK_INT_HEX:
            return "INT_HEX";
        case TOK_INT_DEC:
            return "INT_DEC";
        case TOK_INT_OCT:
            return "INT_OCT";
        case TOK_INT_BIN:
            return "INT_BIN";
        case TOK_COMMA:
            return "COMMA";
        default:
            return "UNKNOWN";
    }
}

dword token_get_int_value(Token* t) {
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
            ZLASM__CRASH("Not int token");
    }
    return 0;
}

char token_get_char_value(char* t) {
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
                ZLASM__CRASH("Invalid escape character");
        }
    }
    return t[0];
}

byte* token_get_string_value(char* val, size_t* __size) {
    byte* res = calloc(strlen(val), sizeof(char));
    for (*__size = 0; val[*__size] != '\0'; *__size += 1) {
        res[*__size] = (byte)val[*__size];
    }
    res = realloc(res, *__size);
    return res;
}

byte* token_get_raw_data(Token* t, size_t* __size) {
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
        {
            dword buf = (dword) token_get_int_value(t);
            res = calloc(1, sizeof(dword));
            memcpy(res, &buf, sizeof(dword));
            *__size = sizeof(dword);
        }
            break;
        default:
            ZLASM__TOKEN_CRASH("Is not data token", t);
            break;
    }
    return res;
}