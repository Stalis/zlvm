//
// Created by Stanislav on 2019-05-07.
//

#ifndef ZLVM_C_LABELTABLE_H
#define ZLVM_C_LABELTABLE_H

#include "../../emulator/include/Types.h"

struct LabelInfo {
    char* name;
    size_t name_size;
    size_t address;
};

struct LabelTable {
    struct LabelInfo* value;
    struct LabelTable* next;
};

void label_table_init(struct LabelTable*);
void label_table_add(struct LabelTable*);

#endif //ZLVM_C_LABELTABLE_H
