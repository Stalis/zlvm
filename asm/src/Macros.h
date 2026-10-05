#ifndef ZLVM_ASM_MACROS_H
#define ZLVM_ASM_MACROS_H

#include "Lexer.h"

/**
 * Captures .macro definitions and returns parser-ready expanded tokens.
 * The returned list and its tokens are owned by the assembler allocation context.
 */
TokenList *macros_expand(TokenList *tokens);

#endif // ZLVM_ASM_MACROS_H
