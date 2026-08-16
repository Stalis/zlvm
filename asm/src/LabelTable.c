#include "LabelTable.h"

#include "Memory.h"

#include <assert.h>
#include <string.h>

void labelTable_init(LabelTable* table) {
    table->value = NULL;
    table->next = NULL;
}

LabelInfo* labelTable_add(LabelTable* table, const char* name) {
    return labelTable_setOrCreate(table, name, 0);
}

LabelInfo* labelTable_setOrCreate(LabelTable* table, const char* name, size_t address) {
    assert(table != NULL);
    assert(name != NULL);

    for (LabelTable* item = table; item != NULL; item = item->next) {
        if (item->value != NULL && strcmp(item->value->name, name) == 0) {
            item->value->address = address;
            return item->value;
        }
    }

    LabelTable* last = table;
    while (last->next != NULL) {
        last = last->next;
    }
    if (last->value != NULL) {
        last->next = asm_calloc(1, sizeof *last->next);
        last = last->next;
    }

    LabelInfo* label = asm_calloc(1, sizeof *label);
    label->name = name;
    label->labelLength = strlen(name);
    label->address = address;
    last->value = label;
    return label;
}

LabelInfo* labelInfo_getIfExist(LabelTable* table, const char* name) {
    assert(table != NULL);
    assert(name != NULL);

    for (LabelTable* item = table; item != NULL; item = item->next) {
        if (item->value != NULL && strcmp(item->value->name, name) == 0) {
            return item->value;
        }
    }
    return NULL;
}

LabelInfo* labelInfo_getOrCreate(LabelTable* table, const char* name) {
    LabelInfo* label = labelInfo_getIfExist(table, name);
    return label == NULL ? labelTable_add(table, name) : label;
}
