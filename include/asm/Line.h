//
// Created by Stanislav on 2019-06-06.
//

#ifndef ZLVM_C_LINE_H
#define ZLVM_C_LINE_H

#include "Directive.h"
#include "Statement.h"

typedef enum LineType {
    L_NONE, L_DIR, L_STMT, L_RAW
} LineType;

typedef struct RawData {
    size_t size;
    byte* data;
} RawData;

typedef struct Line {
    LineType type;
    char* label;
    size_t size;
    union {
        Directive* dir;
        Statement* stmt;
        RawData* raw;
    };
} Line;

#endif //ZLVM_C_LINE_H
