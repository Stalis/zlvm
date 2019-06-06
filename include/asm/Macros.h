//
// Created by Stanislav on 2019-06-03.
//

#ifndef ZLVM_C_MACROS_H
#define ZLVM_C_MACROS_H

#include "Statement.h"

struct MacrosContext {

};

struct Macros {
    const char** params;
    Statement** lines;
};

typedef struct MacrosList {
    struct Macros
} MacrosList;

#endif //ZLVM_C_MACROS_H
