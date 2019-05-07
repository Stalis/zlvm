//
// Created by Stanislav on 2019-05-07.
//

#ifndef ZLVM_C_TOKEN_H
#define ZLVM_C_TOKEN_H

#include "../common/Types.h"

enum TokenType {
    TOK_COMMENT,
    TOK_NEWLINE,

    TOK_ID,
    TOK_REGISTER,
    TOK_LABEL_INIT,
    TOK_LABEL_USE,
    TOK_DIRECTIVE,

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
};

/*
 * Prints token representation to STDOUT
 */
void token_print(struct Token*);

/*
 * Free memory, used by token
 */
void token_free(struct Token*);

#endif //ZLVM_C_TOKEN_H
