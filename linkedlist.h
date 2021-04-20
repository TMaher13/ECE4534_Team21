
#ifndef _LINKED_LIST_
#define _LINKED_LIST_

#include <stdint.h>

typedef struct node {
    uint16_t index;
    float distance;
    struct node *next;
    struct node *last;
} mapNode;

typedef struct list {
    mapNode *head;
    mapNode *tail;
} List;

List* List_makeList();

uint_least8_t List_is_empty(List *list);
void List_add_back(uint16_t index, float distance, List *list);
void List_delete_back(List *list);
void List_destroy(List *list);

#endif
