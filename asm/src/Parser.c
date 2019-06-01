//
// Created by Stanislav on 2019-05-14.
//

#include <stdlib.h>
#include "../include/Parser.h"

#define DEFAULT_LABEL_FORMAT "L_%05lu"
#define DEFAULT_LABEL_FORMAT_SIZE (7)

void parser_init(struct ParserContext* ctx) {
    ctx->lines = malloc(sizeof(struct LineList*));
    line_list_init(ctx->lines);

    ctx->labels = malloc(sizeof(struct LabelList*));
    ctx->labels->value = malloc(sizeof(struct LabelInfo*));
    ctx->labels->next = NULL;

    ctx->lines_count = 0;
    /*
    ctx->sections = malloc(sizeof(struct SectionList*));
    ctx->sections->value = malloc();
    ctx->sections->next = NULL;
    */
}

static struct TokenStream* parser_getLine(struct TokenStream* stream) {
#define NEXT_TOKEN ({                   \
    if (tokenStream_isEof(stream)) {    \
        goto __eof;                     \
    }                                   \
    cur = tokenStream_read(stream);     \
})
    struct TokenList* first = malloc(sizeof(struct TokenList));
    struct TokenList* last = NULL;
    struct Token* cur;
    NEXT_TOKEN;

    while (cur->type == TOK_COMMENT || cur->type == TOK_NEWLINE)
    {
        NEXT_TOKEN;
    }

    first->value = cur;
    last = first;
    NEXT_TOKEN;

    if (first->value->type == TOK_LABEL_INIT && cur->type == TOK_NEWLINE)
    {
        NEXT_TOKEN;
    }

    while (cur->type != TOK_NEWLINE)
    {
        if (cur->type == TOK_COMMA)
        {
            NEXT_TOKEN;
            continue;
        }

        if (cur->type == TOK_COMMENT)
        {
            break;
        }

        last->next = malloc(sizeof(struct TokenList));
        last->next->value = cur;
        last = last->next;
        NEXT_TOKEN;
    }

__eof:
    if (last == NULL)
        return NULL;
    else
        return tokenStream_new(first);
#undef NEXT_TOKEN
}

static struct Statement* parser_readStatement(struct TokenStream* stream, struct Token* cur) {
#define NEXT_TOKEN ({                   \
    if (tokenStream_isEof(stream)) {    \
        goto __eof;                     \
    }                                   \
    cur = tokenStream_read(stream);     \
})

    struct Statement* stmt = malloc(sizeof(struct Statement));
    statement_init(stmt);
    stmt->opcode = cur;
    NEXT_TOKEN;
    if (cur->type == TOK_ID)
    {
        stmt->cond = cur;
        NEXT_TOKEN;
    }
    if (cur->type == TOK_REGISTER)
    {
        stmt->reg1 = cur;
        NEXT_TOKEN;
        if (cur->type == TOK_REGISTER)
        {
            stmt->reg2 = cur;
            NEXT_TOKEN;
        }
    }
    if (
            cur->type == TOK_LABEL_USE ||
            cur->type == TOK_CHAR_LITERAL ||
            cur->type == TOK_INT_BIN ||
            cur->type == TOK_INT_OCT ||
            cur->type == TOK_INT_DEC ||
            cur->type == TOK_INT_HEX
            )
    {
        stmt->imm = cur;
    }

__eof:
    return stmt;
#undef NEXT_TOKEN
}

static struct Directive* parser_readDirective(struct TokenStream* stream, struct Token* cur) {
#define NEXT_TOKEN ({                   \
    if (tokenStream_isEof(stream)) {    \
        goto __eof;                     \
    }                                   \
    cur = tokenStream_read(stream);     \
})

    struct Directive* dir = malloc(sizeof(struct Directive));

    directive_init(dir, cur);
    NEXT_TOKEN;
    while (1)
    {
        directive_add_arg(dir, cur);
        NEXT_TOKEN;
    }

__eof:
    return dir;

#undef NEXT_TOKEN
}

static struct Line* parser_readLine(struct TokenStream* stream) {
#define NEXT_TOKEN ({                   \
    if (tokenStream_isEof(stream)) {    \
        goto __eof;                     \
    }                                   \
    cur = tokenStream_read(stream);     \
})

    struct Line* line = calloc(1, sizeof(struct Line));
    struct Token* cur;

    NEXT_TOKEN;

    if (cur->type == TOK_LABEL_INIT)
    {
        line->label = cur->value;
        NEXT_TOKEN;
    }

    switch (cur->type)
    {
        case TOK_ID:
            line->type = L_STMT;
            line->stmt = parser_readStatement(stream, cur);
            break;
        case TOK_DIRECTIVE:
            line->type = L_DIR;
            line->dir = parser_readDirective(stream, cur);
            break;
        default:
            ZLASM__TOKEN_CRASH("Unexpected token", cur);
    }

__eof:
    return line;
#undef NEXT_TOKEN
}

void parser_addLine(struct ParserContext* ctx, struct Line* line) {
    const char* label = line->label;
    line_list_add(ctx->lines, line);
    labelTable_add(ctx->labels, label);
}

void parser_parse(struct ParserContext* ctx, struct TokenStream* stream) {
    struct TokenStream* tok_line = parser_getLine(stream);
    struct Line* line;

    while (tok_line != NULL)
    {
        line = parser_readLine(tok_line);
        parser_addLine(ctx, line);
        tok_line = parser_getLine(stream);
    }
}

void parser_clear(struct ParserContext* c) {
//    free(ctx->sections);
    line_list_free(c->lines);
    free(c->labels);
}
