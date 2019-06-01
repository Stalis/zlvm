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

struct ParserContext {
    struct LineList* lines;
    size_t lines_count;
};

void parser_init(struct ParserContext*);

void parser_addLine(struct ParserContext*, struct Line*);

void parser_parse(struct ParserContext*, struct TokenStream*);

void parser_clear(struct ParserContext*);

#endif //ZLVM_C_CONTEXT_H
