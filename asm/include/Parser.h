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

enum SectionType {
    S_TEXT, S_DATA
};

struct Section {
    enum SectionType type;
    struct StatementList* stmts;
    struct DirectiveList* directives;
};

struct SectionList {
    struct Section* value;
    struct SectionList* next;
};

struct ParserContext {
    struct LabelTable* labels;
    struct LineList* lines;
    size_t lines_count;
    //struct SectionList* sections;
};

void parser_init(struct ParserContext*);

void parser_readTokens(struct ParserContext* ctx, struct TokenStream* stream);

void parser_procDirectives(struct ParserContext* ctx);

void parser_clear(struct ParserContext*);

#endif //ZLVM_C_CONTEXT_H
