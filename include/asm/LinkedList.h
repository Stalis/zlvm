#ifndef ZLVM_ASM_LINKED_LIST_H
#define ZLVM_ASM_LINKED_LIST_H

#include "../Types.h"

#include <stdlib.h>

typedef struct LinkedListItem {
    void *value;
    struct LinkedListItem *next;
} LinkedListItem;

typedef struct LinkedList {
    LinkedListItem *first;
} LinkedList;

/* Compatibility aliases retained for existing callers. */
typedef LinkedListItem linked_list_item_t;
typedef LinkedList linked_list_t;

static inline LinkedListItem *linked_list_get_last(LinkedList *list) {
    if (list == NULL || list->first == NULL) {
        return NULL;
    }

    LinkedListItem *item = list->first;
    while (item->next != NULL) {
        item = item->next;
    }
    return item;
}

static inline bool linked_list_append(LinkedList *list, void *value) {
    if (list == NULL) {
        return false;
    }

    LinkedListItem *item = calloc(1, sizeof *item);
    if (item == NULL) {
        return false;
    }
    item->value = value;

    LinkedListItem *last = linked_list_get_last(list);
    if (last == NULL) {
        list->first = item;
    } else {
        last->next = item;
    }
    return true;
}

static inline void *linked_list_remove_first(LinkedList *list) {
    if (list == NULL || list->first == NULL) {
        return NULL;
    }

    LinkedListItem *item = list->first;
    list->first = item->next;
    void *value = item->value;
    free(item);
    return value;
}

static inline void *linked_list_remove_last(LinkedList *list) {
    if (list == NULL || list->first == NULL) {
        return NULL;
    }

    LinkedListItem *previous = NULL;
    LinkedListItem *item = list->first;
    while (item->next != NULL) {
        previous = item;
        item = item->next;
    }

    if (previous == NULL) {
        list->first = NULL;
    } else {
        previous->next = NULL;
    }

    void *value = item->value;
    free(item);
    return value;
}

static inline size_t linked_list_count(const LinkedList *list) {
    size_t count = 0;
    if (list == NULL) {
        return count;
    }

    for (const LinkedListItem *item = list->first; item != NULL; item = item->next) {
        count++;
    }
    return count;
}

#endif // ZLVM_ASM_LINKED_LIST_H
