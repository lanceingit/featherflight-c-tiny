#pragma once

struct list_struct 
{
    void* node;
    struct list_struct* next;
};

typedef struct list_struct List;

void list_init(List* head);
void list_del(List* head, uint16_t num);
void list_insert(List* head, void* node, uint16_t num);
void list_add(List* head, void* node);

