//
// Created by Stanislav on 2019-05-14.
//

#ifndef ZLVM_C_LINELIST_H
#define ZLVM_C_LINELIST_H

#include "../../emulator/include/Types.h"
#include "Directive.h"
#include "Statement.h"

enum LineType {
    L_NONE, L_DIR, L_STMT, L_RAW
};

struct RawData {
    size_t size;
    byte* data;
};

struct Line {
    enum LineType type;
    char* label;
    size_t size;
    union {
        struct Directive* dir;
        struct Statement* stmt;
        struct RawData* raw;
    };
};

struct LineList {
    struct Line* value;
    struct LineList* next;
};

void line_list_init(struct LineList*);
void line_list_add(struct LineList*);
void line_list_free(struct LineList*);

void line_print(struct Line*);

#endif //ZLVM_C_LINELIST_H
