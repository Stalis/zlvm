//
// Created by Stanislav on 2019-06-01.
//

#ifndef ZLVM_C_ASSEMBLER_H
#define ZLVM_C_ASSEMBLER_H

#include "Parser.h"

typedef struct AssemblerContext {
    struct LabelTable* labels;
    const char* entry;
    const char** globals;
    size_t globalsCount;
    const char** externals;
    size_t externalsCount;
    LineList* lines;
} AssemblerContext;

void asm_init(AssemblerContext* ctx);
void asm_processDirectives(AssemblerContext* ctx, ParserContext* parser);
void asm_processLabels(AssemblerContext* ctx);
byte* asm_translate(AssemblerContext* ctx, size_t* __size);
void asm_addGlobal(AssemblerContext* ctx, const char* sym);
void asm_addExternal(AssemblerContext* ctx, const char* sym);

#endif //ZLVM_C_ASSEMBLER_H
