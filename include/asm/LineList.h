//
// Created by Stanislav on 2019-05-14.
//

#ifndef ZLVM_C_LINELIST_H
#define ZLVM_C_LINELIST_H

#include "../Types.h"
#include "Line.h"

typedef struct LineList {
    struct Line* value;
    struct LineList* next;
} LineList;

typedef struct LineStream {
    LineList* first;
} LineStream;

void line_list_init(LineList*);
void line_list_add(LineList* lst, Line* line);
void line_list_free(LineList*);

void line_print(Line*);

LineStream* lineStream_new(LineList*);
Line* lineStream_read(LineStream*);
#endif //ZLVM_C_LINELIST_H
