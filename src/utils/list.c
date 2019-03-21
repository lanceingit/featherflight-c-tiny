#include "board.h"
#include "list.h"
#include "mm.h"


#define LIST_FOREACH(pos, head) \
    for(pos=head; pos!=NULL; pos=pos->next)


list_s* list_find_by_num(list_s* head, uint16_t num)
{
    uint16_t cnt=0;
    list_s* pos;
    LIST_FOREACH(pos, head) {
        if(num == cnt) {
            return pos;
        }        
    }
    
    return NULL;
}

void list_del(list_s* head, uint16_t num)
{
    if(num == 0) {
        head = head->next;
    } else {
        uint16_t cnt=0;
        list_s* pos;
        LIST_FOREACH(pos, head) {
            if(num-1 == cnt) {
                pos->next = pos->next->next;
            }
            cnt++;
        }
    }
}

void list_insert(list_s* head, void* node, uint16_t num)
{
    list_s* new = mm_malloc(sizeof(list_s));
    if(new != NULL) {
        uint16_t cnt=0;
        list_s* pos;
        LIST_FOREACH(pos, head) {
            if(num == cnt) {
                new->node = node;
                new->next = pos->next;
                pos->next = new;
                return;
            }
            cnt++;
        }
    }
}

void list_add(list_s* head, void* node)
{
    list_s* new = mm_malloc(sizeof(list_s));
    if(new != NULL) {
        list_s* pos;
        LIST_FOREACH(pos, head) {
            if(pos->next == NULL) {
                new->node = node;
                pos->next = new;
                new->next = NULL;
            }
        }
    }
}

void list_init(list_s* head)
{
    head->next = NULL;
}

