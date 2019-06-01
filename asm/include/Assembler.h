//
// Created by Stanislav on 2019-06-01.
//

#ifndef ZLVM_C_ASSEMBLER_H
#define ZLVM_C_ASSEMBLER_H

#include "Parser.h"

struct AssemblerContext {
    struct LabelTable* labels;
    const char** globals;
    const char** externals;
};

void process_directives(struct ParserContext* ctx);

#endif //ZLVM_C_ASSEMBLER_H
