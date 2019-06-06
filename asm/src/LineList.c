// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
// Created by Stanislav on 2019-05-14.
//

#include <printf.h>
#include <assert.h>

#include "LineList.h"
#include "Memory.h"

static struct LineList* lineList_getLast(struct LineList* l) {
    assert(l != NULL);
    while (l->next != NULL)
    {
        l = l->next;
    }
    return l;
}

void line_list_init(struct LineList* lst) {
    lst->value = NULL;
    lst->next = NULL;
}

void line_list_add(struct LineList* lst, struct Line* line) {
    lst = lineList_getLast(lst);
    if (lst->value == NULL)
    {
        lst->value = line;
        return;
    }

    lst->next = calloc(1, sizeof(struct LineList));
    lst->next->value = line;
}

void line_list_free(struct LineList* lst) {
    if (lst != NULL)
    {
        if (lst->value != NULL)
        {
            free(lst->value);
        }
        if (lst->next != NULL)
        {
            line_list_free(lst->next);
        }
        free(lst);
    }
}

void line_print(struct Line* l) {
    printf("%s: ", l->label);
    if (l->type == L_NONE)
    {
        printf("NONE");
    }
    else if (l->type == L_DIR)
    {
        size_t len = 0;
        for (size_t i = 0; i < l->dir->argc; i++)
        {
            len += l->dir->argv[i]->size;
            len += 1;
        }

        printf(".%d", l->dir->type);
        for (size_t i = 0; i < l->dir->argc; i++)
        {
            if (i != 0) printf(",");
            printf(" %s", l->dir->argv[i]->value);
        }
    }
    else if (l->type == L_STMT)
    {
        printf("%s %s ",
               l->stmt->opcode->value,
               NULL == l->stmt->cond ? "un" : l->stmt->cond->value);
        printf("$%s, $%s, %s",
               NULL == l->stmt->reg1 ? "zero" : l->stmt->reg1->value,
               NULL == l->stmt->reg2 ? "zero" : l->stmt->reg2->value,
               NULL == l->stmt->imm ? "NONE" : l->stmt->imm->value
        );
    }
    else if (l->type == L_RAW)
    {
        printf("RAW DATA(%lu) [ ", l->raw->size);
        for (size_t i = 0; i < l->raw->size; i++)
        {
            printf("0x%X ", l->raw->data[i]);
        }
        printf("]");
    }
    printf("\n");
}

LineStream* lineStream_new(LineList* lst) {
    LineStream* stream = malloc(sizeof(LineStream));
    stream->first = lst;
    return stream;
}

Line* lineStream_read(LineStream* stream) {
    if (stream->first == NULL)
        return NULL;

    LineList* buf = stream->first;
    stream->first = buf->next;
    Line* val = buf->value;
    buf->value = NULL;
    buf->next = NULL;
    return val;
}