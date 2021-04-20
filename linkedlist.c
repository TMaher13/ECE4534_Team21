#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <linkedlist.h>

mapNode* createNode(uint16_t index, float distance);

mapNode* createNode(uint16_t index, float distance) {
    mapNode *newNode = malloc(sizeof(mapNode));
    if(!newNode)
        return NULL;

    newNode->index = index;
    newNode->distance = distance;
    newNode->next = NULL;
    newNode->last = NULL;

    return newNode;
}

List* List_makeList() {
    List *list = malloc(sizeof(List));
    if(!list)
        return NULL;

    list->head = NULL;
    list->tail = NULL;
    return list;
}

uint_least8_t List_is_empty(List *list) {
    return list->head == NULL;
}

void List_add_back(uint16_t index, float distance, List *list) {
    mapNode *current = createNode(index, distance);

    if(list->head == NULL) {
        list->head = current;
        list->tail = current;
    }
    else {
        if(list->tail != NULL) {
            list->tail->next = current;
        }

        current->next = NULL;
        current->last = list->tail;
        list->tail = current;
    }
}


void List_delete_back(List *list) {
    mapNode *temp;
    if(list->tail == NULL) {
        return;
    }
    else {
        temp = list->tail;
        list->tail = list->tail->last;
        if(list->tail != NULL) {
            list->tail->next = NULL;
        }
        else {
            list->head = NULL;
        }
        free(temp);
    }
}


void List_destroy(List *list) {
    while(!List_is_empty(list))
        List_delete_back(list);
}








