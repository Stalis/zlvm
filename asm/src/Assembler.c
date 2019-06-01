//
// Created by Stanislav on 2019-06-01.
//

#include "../include/Assembler.h"

static const char* CONTEXT_DELIMITER = ".";

static const char* set_label_context(const char* ctx, const char* label);

void process_directives(struct ParserContext* ctx) {
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
        if (proc_context != NULL && line->value->label != NULL)
        {
            void* buf = line->value->label;
            line->value->label = set_label_context(proc_context, line->value->label);
        }
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
                    line->next->value->label = "";
                    proc_context = line->value->dir->argv[0]->value;
                    REMOVE_LINE;
                    break;
                case DIR_ENDPROC:
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

static const char* set_label_context(const char* ctx, const char* label) {
    if (strlen(label) == 0)
    {
        return strdup(ctx);
    }
    char* buf = strdup(ctx);
    strcat(buf, CONTEXT_DELIMITER);
    strcat(buf, label);
    return buf;
}