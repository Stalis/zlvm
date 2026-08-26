#ifndef ZLVM_ASM_MACROS_H
#define ZLVM_ASM_MACROS_H

#include "Statement.h"

typedef struct MacrosContext {
    size_t macro_count;
} MacrosContext;

typedef struct Macros {
    const char **params;
    Statement **lines;
} Macros;

typedef struct MacrosList {
    Macros value;
    struct MacrosList *next;
} MacrosList;

#endif // ZLVM_ASM_MACROS_H
