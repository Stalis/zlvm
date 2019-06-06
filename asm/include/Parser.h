//
// Created by Stanislav on 2019-05-14.
//

#ifndef ZLVM_C_CONTEXT_H
#define ZLVM_C_CONTEXT_H

#include "LineList.h"
#include "LabelTable.h"
#include "Statement.h"
#include "Directive.h"
#include "Lexer.h"
#include "Token.h"

typedef struct ParserContext {
    struct LineList* lines;
    size_t lines_count;
} ParserContext;

void parser_init(ParserContext*);

void parser_addLine(ParserContext*, Line*);

void parser_parse(ParserContext*, TokenStream*);

void parser_clear(ParserContext*);

#endif //ZLVM_C_CONTEXT_H
