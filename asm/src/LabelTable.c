// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
// Created by Stanislav on 2019-05-07.
//

#include <assert.h>
#include <string.h>
#include "LabelTable.h"
#include "Memory.h"

void labelTable_init(struct LabelTable* t) {
    t->value = NULL;
    t->next = NULL;
}

struct LabelInfo* labelTable_add(struct LabelTable* t, const char* name) {
    return labelTable_setOrCreate(t, name, 0);
}

struct LabelInfo* labelTable_setOrCreate(struct LabelTable* t, const char* name, size_t addr) {
    assert(t != NULL);
    struct LabelInfo* l = calloc(1, sizeof(struct LabelInfo));
    l->name = name;
    l->address = addr;

    struct LabelTable* last = t;

    while (last->next != NULL)
    {
        last = last->next;
    }

    if (last->value != NULL)
    {
        last->next = calloc(1, sizeof(struct LabelTable));
        last = last->next;
    }

    last->value = l;

    return last->value;
}

struct LabelInfo* labelInfo_getIfExist(struct LabelTable* t, const char* name) {
    assert(t != NULL);
    if (t->value == NULL) return NULL;

    while (1)
    {
        if (0 == strcmp(t->value->name, name))
        {
            return t->value;
        }
        if (t->next != NULL)
        {
            t = t->next;
        }
        else
        {
            return NULL;
        }
    }
}

struct LabelInfo* labelInfo_getOrCreate(struct LabelTable* t, const char* name) {
    struct LabelInfo* res = labelInfo_getIfExist(t, name);
    if (res == NULL)
    {
        res = labelTable_add(t, name);
    }
    return res;
}