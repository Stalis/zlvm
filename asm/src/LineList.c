// Created by Stanislav on 2019-05-14.
//

#include "LineList.h"

#include "Memory.h"

#include <assert.h>
#include <stdio.h>

static LineList* line_list_get_last(LineList* list) {
    assert(list != NULL);
    while (list->next != NULL) {
        list = list->next;
    }
    return list;
}

void line_list_init(LineList* list) {
    list->value = NULL;
    list->next = NULL;
}

void line_list_add(LineList* list, Line* line) {
    list = line_list_get_last(list);
    if (list->value == NULL) {
        list->value = line;
        return;
    }

    list->next = asm_calloc(1, sizeof *list->next);
    list->next->value = line;
}

void line_list_free(LineList* list) {
    if (list != NULL) {
        if (list->value != NULL) {
            asm_free(list->value);
        }
        if (list->next != NULL) {
            line_list_free(list->next);
        }
        asm_free(list);
    }
}

void line_print(Line* line) {
    printf("%s: ", line->label);
    if (line->type == L_NONE) {
        printf("NONE");
    } else if (line->type == L_DIR) {
        printf(".%d", line->dir->type);
        for (size_t i = 0; i < line->dir->argc; i++) {
            if (i != 0) {
                printf(",");
            }
            printf(" %s", line->dir->argv[i]->value);
        }
    } else if (line->type == L_STMT) {
        printf("%s %s ", line->stmt->opcode->value,
               line->stmt->cond == NULL ? "un" : line->stmt->cond->value);
        printf("$%s, $%s, %s", line->stmt->reg1 == NULL ? "zero" : line->stmt->reg1->value,
               line->stmt->reg2 == NULL ? "zero" : line->stmt->reg2->value,
               line->stmt->imm == NULL ? "NONE" : line->stmt->imm->value);
    } else if (line->type == L_RAW) {
        printf("RAW DATA(%zu) [ ", line->raw->size);
        for (size_t i = 0; i < line->raw->size; i++) {
            printf("0x%X ", line->raw->data[i]);
        }
        printf("]");
    }
    printf("\n");
}

LineStream* lineStream_new(LineList* list) {
    LineStream* stream = asm_malloc(sizeof *stream);
    stream->first = list;
    return stream;
}

Line* lineStream_read(LineStream* stream) {
    if (stream->first == NULL) {
        return NULL;
    }

    LineList* item = stream->first;
    stream->first = item->next;
    Line* line = item->value;
    asm_free(item);
    return line;
}
