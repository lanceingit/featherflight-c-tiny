#pragma once

struct list_s 
{
    void* node;
    struct list_s* next;
};

void list_init(struct list_s* head);
void list_del(struct list_s* head, uint16_t num);
void list_insert(struct list_s* head, void* node, uint16_t num);
void list_add(struct list_s* head, void* node);

