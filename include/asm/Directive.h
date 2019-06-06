//
// Created by Stanislav on 2019-05-07.
//

#ifndef ZLVM_C_DIRECTIVE_H
#define ZLVM_C_DIRECTIVE_H

#include "../Types.h"
#include "Token.h"

typedef enum DirectiveType {
    DIR_SECTION,
    DIR_GLOBAL,
    DIR_EXTERN,
    DIR_ALIGN,
    DIR_ENTRY,
    DIR_LOCATE,
    /* Data directives */
    DIR_ASCII, // Non-null-terminated string
    DIR_ASCIIZ, // Null-terminated string
    DIR_BYTE, // sequence of byte(8 bit) values
    DIR_HWORD, // sequence of half word(16 bit) values
    DIR_WORD, // sequence of word(32 bit) values
    DIR_DWORD, // sequence of double word(64 bit) values
    DIR_SPACE, // empty space for N bytes
    /* Other directives */
    DIR_MACRO,
    DIR_ENDMACRO,
    DIR_PROC,
    DIR_ENDPROC,
    DIR_TOTAL,
} DirectiveType;

typedef struct Directive {
    DirectiveType type;
    size_t argc;
    Token** argv;
} Directive;

/**
 * @brief   Initialize directive with name as token
 *          If name token is valid, sets directive.type
 * @return  `true` if directive name is valid, else returns `false`
 */
bool directive_init(Directive*, Token* name);

/**
 * @brief Free memory, allocated for directive
 */
void directive_free(Directive*);

/**
 * @brief Adds argument token to directive statement
 */
void directive_add_arg(Directive*, Token*);

/**
 * @brief Returns pointer to data directive raw data
 * @param __size pointer to write data size value
 */
uint8_t* directive_get_raw_data(Directive*, size_t* __size);

/**
 * @param t type of directive
 * @return `true` if t is data directive type, else `false`
 */
static inline bool is_data_directive(DirectiveType t) {
    switch (t)
    {
        case DIR_ASCII:
        case DIR_ASCIIZ:
        case DIR_BYTE:
        case DIR_HWORD:
        case DIR_WORD:
        case DIR_DWORD:
        case DIR_SPACE:
            return true;
        default:
            return false;
    }
}

#endif //ZLVM_C_DIRECTIVE_H
