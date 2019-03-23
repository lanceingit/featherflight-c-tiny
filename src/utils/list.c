#include "board.h"
#include "list.h"
#include "mm.h"


#define LIST_FOREACH(pos, head) \
    for(pos=head; pos!=NULL; pos=pos->next)


List* list_find_by_num(List* head, uint16_t num)
{
    uint16_t cnt=0;
    List* pos;
    LIST_FOREACH(pos, head) {
        if(num == cnt) {
            return pos;
        }        
    }
    
    return NULL;
}

void list_del(List* head, uint16_t num)
{
    if(num == 0) {
        head = head->next;
    } else {
        uint16_t cnt=0;
        List* pos;
        LIST_FOREACH(pos, head) {
            if(num-1 == cnt) {
                pos->next = pos->next->next;
            }
            cnt++;
        }
    }
}

void list_insert(List* head, void* node, uint16_t num)
{
    List* new = mm_malloc(sizeof(List));
    if(new != NULL) {
        uint16_t cnt=0;
        List* pos;
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

void list_add(List* head, void* node)
{
    List* new = mm_malloc(sizeof(List));
    if(new != NULL) {
        List* pos;
        LIST_FOREACH(pos, head) {
            if(pos->next == NULL) {
                new->node = node;
                pos->next = new;
                new->next = NULL;
            }
        }
    }
}

void list_init(List* head)
{
    head->next = NULL;
}

