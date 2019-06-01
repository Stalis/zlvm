//
// Created by Stanislav on 2019-05-14.
//

#include <stdlib.h>
#include "../include/Parser.h"

#define DEFAULT_LABEL_FORMAT "L_%05lu"
#define DEFAULT_LABEL_FORMAT_SIZE (7)

static struct LineList* get_last_line(struct LineList*);

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

void parser_readTokens(struct ParserContext* ctx, struct TokenStream* stream) {
#define NEWLINE \
({                                      \
    if (line->value->type != L_NONE)    \
    {                                   \
        if (line->value->label == NULL) \
        {                               \
            line->value->label = calloc(DEFAULT_LABEL_FORMAT_SIZE + 1, sizeof(char)); \
            sprintf(line->value->label, DEFAULT_LABEL_FORMAT, ctx->lines_count);      \
        }                               \
        line_list_add(line);            \
        line = line->next;              \
        ctx->lines_count++;             \
    }                                   \
})

#define NEXT_TOKEN ({                   \
    if (tokenStream_isEof(stream)) {    \
        goto __end;                     \
    }                                   \
    cur = tokenStream_read(stream);     \
})

    struct Token* cur = tokenStream_read(stream);
    struct LineList* line = get_last_line(ctx->lines);

    while (!tokenStream_isEof(stream))
    {
        switch (cur->type)
        {
            case TOK_LABEL_INIT:
                line->value->label = strdup(cur->value);
                line->value->size = cur->size;
                break;
            case TOK_COMMENT:
                break;
            case TOK_NEWLINE:
                if (line->value->type != L_NONE)
                {
                    if (line->value->label == NULL)
                    {
                        line->value->label = calloc(DEFAULT_LABEL_FORMAT_SIZE + 1, sizeof(char));
                        sprintf(line->value->label, DEFAULT_LABEL_FORMAT, ctx->lines_count);
                    }
                }
                else
                {
                    NEWLINE;
                }
                break;

            case TOK_ID:
                if (line->value->type == L_NONE)
                {
                    struct Statement stmt;
                    statement_init(&stmt);
                    stmt.opcode = cur;
                    NEXT_TOKEN;
                    if (cur->type == TOK_COMMA)
                        NEXT_TOKEN;
                    if (cur->type == TOK_ID)
                    {
                        stmt.cond = cur;
                        NEXT_TOKEN;
                        if (cur->type == TOK_COMMA)
                            NEXT_TOKEN;
                    }
                    if (cur->type == TOK_REGISTER)
                    {
                        stmt.reg1 = cur;
                        NEXT_TOKEN;
                        if (cur->type == TOK_COMMA)
                            NEXT_TOKEN;
                        if (cur->type == TOK_REGISTER)
                        {
                            stmt.reg2 = cur;
                            NEXT_TOKEN;
                            if (cur->type == TOK_COMMA)
                                NEXT_TOKEN;
                        }
                    }
                    switch (cur->type)
                    {
                        case TOK_STRING_LITERAL:
                        case TOK_CHAR_LITERAL:
                        case TOK_INT_HEX:
                        case TOK_INT_DEC:
                        case TOK_INT_OCT:
                        case TOK_INT_BIN:
                        case TOK_LABEL_USE:
                            stmt.imm = cur;
                            NEXT_TOKEN;
                            break;
                        default:
                            break;
                    }
                    while (cur->type != TOK_NEWLINE)
                    {
                        NEXT_TOKEN;
                    }
                    line->value->type = L_STMT;
                    struct Statement* ptr = malloc(sizeof(struct Statement));
                    memcpy(ptr, &stmt, sizeof(struct Statement));
                    line->value->stmt = ptr;
                    NEWLINE;
                }
                break;

            case TOK_DIRECTIVE:
                if (line->value->type == L_NONE)
                {
                    struct Directive dir;
                    directive_init(&dir, cur);

                    NEXT_TOKEN;
                    while (cur->type != TOK_NEWLINE && cur->type != TOK_COMMENT)
                    {
                        directive_add_arg(&dir, cur);
                        NEXT_TOKEN;
                        if (cur->type == TOK_COMMA)
                            NEXT_TOKEN;
                    }

                    line->value->type = L_DIR;
                    struct Directive* ptr = malloc(sizeof(struct Directive));
                    memcpy(ptr, &dir, sizeof(struct Directive));
                    line->value->dir = ptr;
                    NEWLINE;
                }
                break;
            default:
                ZLASM__TOKEN_CRASH("Unexpected token", cur);
                break;
        }

        NEXT_TOKEN;
    }
__end:
    free(stream);

#undef NEXT_TOKEN
#undef NEWLINE
}

void parser_procDirectives(struct ParserContext* ctx) {
    struct LineList* prev_line = NULL;
    struct LineList* line = ctx->lines;
    char* proc_context = NULL;

#define REMOVE_LINE \
    prev_line->next = line->next; \
    line->next = NULL; \
    line_list_free(line); \
    line = prev_line;

    while (line != NULL)
    {
        if (line->value->type == L_DIR)
        {
            if (is_data_directive(line->value->dir->type))
            {
                struct Directive* dir = line->value->dir;
                line->value->type = L_RAW;
                line->value->raw = calloc(1, sizeof(struct RawData));
                line->value->raw->data = directive_get_raw_data(dir, &line->value->raw->size);
                continue;
            }
            switch (line->value->dir->type)
            {
                case DIR_SECTION:
                    // TODO: Add sections
                    break;
                case DIR_GLOBAL:
                    // TODO: Add global symbols
                    break;
                case DIR_EXTERN:
                    // TODO: Add extern symbols
                    break;
                case DIR_ALIGN:
                    // TODO: Add aligning data
                    break;
                case DIR_ENTRY:
                    // TODO: Add entry point data? Oo
                    break;
                case DIR_LOCATE:
                    // TODO: Add static location shift
                    break;
                case DIR_PROC:
                    line->next->value->label = strdup(line->value->dir->argv[0]->value);
                    line->next->value->size = line->value->dir->argv[0]->size;
                    proc_context = line->value->dir->argv[0]->value;
                    REMOVE_LINE;
                    break;
                case DIR_ENDPROC:
                    free(prev_line->value->label);
                    prev_line->value->label = strcat(proc_context, ".end");
                    proc_context = NULL;
                    REMOVE_LINE;
                    break;
                case DIR_MACRO:
                    // TODO: Add macroses
                    break;
                case DIR_ENDMACRO:
                    break;
                default:
                    ZLASM__CRASH("Invalid directive");
            }
        }
        prev_line = line;
        line = line->next;
    }
#undef REMOVE_LINE
}

void parser_clear(struct ParserContext* c) {
//    free(ctx->sections);
    line_list_free(c->lines);
    free(c->labels);
}

static struct LineList* get_last_line(struct LineList* lst) {
    if (lst == NULL)
        return NULL;
    else if (lst->next == NULL)
        return lst;
    else
    {
        struct LineList* buf = lst->next;
        while (buf->next != NULL)
        {
            buf = buf->next;
        }
        return buf;
    }
}
