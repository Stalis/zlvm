//
// Created by Stanislav on 2019-06-01.
//

#ifndef ZLVM_C_ASSEMBLER_H
#define ZLVM_C_ASSEMBLER_H

#include "Parser.h"

typedef struct AssemblerContext {
    struct LabelTable *labels;
    const char *entry;
    const char **globals;
    size_t globalsCount;
    const char **externals;
    size_t externalsCount;
    LineList *lines;
} AssemblerContext;

void asm_init(AssemblerContext *context);
void asm_processDirectives(AssemblerContext *context, ParserContext *parser);
void asm_processLabels(AssemblerContext *context);
byte *asm_translate(AssemblerContext *context, size_t *output_size);
void asm_addGlobal(AssemblerContext *context, const char *symbol);
void asm_addExternal(AssemblerContext *context, const char *symbol);

#endif // ZLVM_C_ASSEMBLER_H
