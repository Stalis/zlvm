//
// Created by Stanislav on 2019-05-14.
//

#include <printf.h>
#include "../include/LineList.h"

void line_list_init(struct LineList* lst) {
    lst->value = calloc(1, sizeof(struct Line*));
    lst->next = NULL;
}

void line_list_add(struct LineList* lst) {
    lst->next = calloc(1, sizeof(struct LineList*));
    line_list_init(lst->next);
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
    if (l->type == L_DIR)
    {

    }
    else if (l->type == L_STMT)
    {

    }
    else
    {

    }
}