// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
// Created by Stanislav on 2019-05-07.
//

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