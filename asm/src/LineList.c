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
        size_t len = 0;
        for (size_t i = 0; i < l->dir->argc; i++)
        {
            len += l->dir->argv[i]->size;
            len += 1;
        }

        printf("%s: .%d", l->label, l->dir->type);
        for (size_t i = 0; i < l->dir->argc; i++)
        {
            printf(" %s", l->dir->argv[i]->value);
        }
        printf("\n");
    }
    else if (l->type == L_STMT)
    {
        printf("%s: %s %s ",
               l->label,
               l->stmt->opcode->value,
               NULL == l->stmt->cond ? "un" : l->stmt->cond->value);
        printf("$%s, $%s, %s\n",
               NULL == l->stmt->reg1 ? "zero" : l->stmt->reg1->value,
               NULL == l->stmt->reg2 ? "zero" : l->stmt->reg2->value,
               NULL == l->stmt->imm ? "NONE" : l->stmt->imm->value
        );
    }
    else
    {

    }
}