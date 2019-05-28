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
#define THROW_ERROR(err) {\
    char* msg = strcat(err, cur->value); \
    ZLASM__TOKEN_CRASH(msg, cur); }

    struct Token* cur = tokenStream_read(stream);
    struct LineList* line = get_last_line(ctx->lines);

    while (cur != NULL && cur->value != NULL)
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
                    line_list_add(line);
                    line = line->next;
                    ctx->lines_count++;
                }
                break;
            case TOK_ID:
                if (line->value->type == L_NONE)
                {
                    line->value->type = L_STMT;
                    line->value->stmt = calloc(1, sizeof(struct Statement*));
                    line->value->stmt->opcode = cur;
                }
                else if (line->value->type == L_STMT)
                {
                    if (line->value->stmt->cond == NULL)
                    {
                        line->value->stmt->cond = cur;
                    }
                    else THROW_ERROR("Invalid condition using: ");
                }
                else if (line->value->type == L_DIR)
                {
                    directive_add_arg(line->value->dir, cur);
                }
                break;

            case TOK_REGISTER:
                if (line->value->type == L_STMT)
                {
                    if (line->value->stmt->reg1 == NULL)
                        line->value->stmt->reg1 = cur;
                    else if (line->value->stmt->reg2 == NULL)
                        line->value->stmt->reg1 = cur;
                    else THROW_ERROR("Invalid register using: ");
                }
                break;

            case TOK_DIRECTIVE:
                if (line->value->type == L_NONE)
                {
                    line->value->type = L_DIR;
                    line->value->dir = calloc(1, sizeof(struct Directive*));
                    directive_init(line->value->dir, cur);
                }
                break;

            case TOK_COMMA:
                break;

            case TOK_STRING_LITERAL:
            case TOK_CHAR_LITERAL:
            case TOK_INT_HEX:
            case TOK_INT_DEC:
            case TOK_INT_OCT:
            case TOK_INT_BIN:
            case TOK_LABEL_USE:
                if (line->value->type == L_STMT)
                {
                    if (line->value->stmt->imm == NULL)
                        line->value->stmt->imm = cur;
                    else THROW_ERROR("Invalid immediate using: ");
                }
                else if (line->value->type == L_DIR)
                    directive_add_arg(line->value->dir, cur);
                else THROW_ERROR("Invalid immediate using: ");
                break;
        }

        cur = tokenStream_read(stream);
    }
    free(stream);
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

    while (line->next != NULL)
    {
        if (line->value->type == L_DIR)
        {
            if (is_data_directive(line->value->dir->type))
            {
                line->value->type = L_RAW;
                line->value->raw = calloc(1, sizeof(struct RawData));
                line->value->raw->data = directive_get_raw_data(line->value->dir, &line->value->raw->size);
                line->value->dir = NULL;
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
                    REMOVE_LINE;
                    break;
                case DIR_ENDPROC:
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
