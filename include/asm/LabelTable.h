//
// Created by Stanislav on 2019-05-07.
//

#ifndef ZLVM_C_LABELTABLE_H
#define ZLVM_C_LABELTABLE_H

#include "../Types.h"

typedef struct LabelInfo {
    const char* name;
    size_t labelLength;
    size_t address;
} LabelInfo;

typedef struct LabelTable {
    struct LabelInfo* value;
    struct LabelTable* next;
} LabelTable;

void labelTable_init(LabelTable*);
struct LabelInfo* labelTable_add(LabelTable*, const char* name);
struct LabelInfo* labelTable_setOrCreate(LabelTable*, const char* name, size_t addr);
struct LabelInfo* labelInfo_getIfExist(LabelTable*, const char* name);
struct LabelInfo* labelInfo_getOrCreate(LabelTable*, const char* name);

#endif //ZLVM_C_LABELTABLE_H
