#pragma once

struct list_struct 
{
    void* node;
    struct list_struct* next;
};

typedef struct list_struct list_s;

void list_init(list_s* head);
void list_del(list_s* head, uint16_t num);
void list_insert(list_s* head, void* node, uint16_t num);
void list_add(list_s* head, void* node);

