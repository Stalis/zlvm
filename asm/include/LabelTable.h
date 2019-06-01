//
// Created by Stanislav on 2019-05-07.
//

#ifndef ZLVM_C_LABELTABLE_H
#define ZLVM_C_LABELTABLE_H

#include "../../emulator/include/Types.h"

struct LabelInfo {
    char* name;
    size_t address;
};

struct LabelTable {
    struct LabelInfo* value;
    struct LabelTable* next;
};

void labelTable_init(struct LabelTable*);
void labelTable_add(struct LabelTable*, const char* name);
struct LabelInfo* labelInfo_getIfExist(struct LabelTable*, const char* name);
struct LabelInfo* labelInfo_getOrCreate(struct LabelTable*, const char* name);

#endif //ZLVM_C_LABELTABLE_H
