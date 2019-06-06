//
// Created by Stanislav on 2019-05-30.
//

#ifndef ZLVM_C_LINKEDLIST_H
#define ZLVM_C_LINKEDLIST_H

#include "../Types.h"

typedef struct LinkedListItem {
    void* value;
    struct LinkedListItem* next;
} linked_list_item_t;

typedef struct LinkedList {
    struct LinkedListItem* first;
} linked_list_t;

static struct LinkedListItem* linkedList_getLast(struct LinkedList* l) {
    if (l->first == NULL)
    {
        return NULL;
    }

    struct LinkedListItem* item = l->first;
    while (item->next != NULL)
    {
        item = item->next;
    }
    return item;
}

static void linkedList_append(struct LinkedList* l, void* value) {
    struct LinkedListItem* last = linkedList_getLast(l);

    if (last == NULL)
    {
        l->first = calloc(1, sizeof(struct LinkedListItem));
        last = l->first;
    }
    else
    {
        last->next = calloc(1, sizeof(struct LinkedListItem));
        last = last->next;
    }

    last->value = value;
}

static void* linkedList_removeFirst(struct LinkedList* l) {
    if (l->first == NULL)
    {
        return NULL;
    }

    void* value = l->first->value;
    void* buf = l->first;

    l->first = l->first->next;
    free(buf);

    return value;
}

static void* linkedList_removeLast(struct LinkedList* l) {
    if (l->first == NULL)
    {
        return NULL;
    }

    struct LinkedListItem* prev = NULL;
    struct LinkedListItem* buf = l->first;
    while (buf->next != NULL)
    {
        prev = buf;
        buf = buf->next;
    }

    if (prev != NULL)
    {
        prev->next = NULL;
    }
    else
    {
        // If previous in NULL - buf is first element
        l->first = NULL;
    }

    void* value = buf->value;
    free(buf);

    return value;
}

static size_t linkedList_count(struct LinkedList* l) {
    if (l->first == NULL)
    {
        return 0;
    }

    size_t count = 0;
    struct LinkedListItem* buf = l->first;
    while (buf->next != NULL)
    {
        buf = buf->next;
        count++;
    }

    return count;
}

#endif //ZLVM_C_LINKEDLIST_H
