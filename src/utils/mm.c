/**                                               _____           ,-.
 * _______       _____       _____                ___   _,.      /  /
 * ___    |__   ____(_)_____ __  /______________  __   ; \____,-==-._  )
 * __  /| |_ | / /_  /_  __ `/  __/  __ \_  ___/  _    //_    `----' {+>
 * _  ___ |_ |/ /_  / / /_/ // /_ / /_/ /  /      _    `  `'--/  /-'`(
 * /_/  |_|____/ /_/  \__,_/ \__/ \____//_/       _          /  /
 *                                                           `='
 *
 * mm.c
 *
 * v1.0
 *
 * Dynamic memory management, reducing memory fragmentation
 */
#include "board.h"
#include "mm.h"
#include "debug.h"

#define MM_MODULE_STATIC            1
#define MM_MODULE_DYNAMIC_ADDR      2
#define MM_MODULE_DYNAMIC_SIZE      3

#define USE_MM  MM_MODULE_DYNAMIC_ADDR

#define BYTE_ALIGNMENT_MASK         (MM_BYTE_ALIGNMENT-1)


struct block_s {
    struct block_s* next;
    size_t size;
};

//#define BLOCK_STRUCT_SIZE	   (sizeof(BlockLink_t) + ((size_t) (MM_BYTE_ALIGNMENT-1))) & ~((size_t)BYTE_ALIGNMENT_MASK)
#define BLOCK_STRUCT_SIZE	   ((sizeof(struct block_s)+((size_t)(MM_BYTE_ALIGNMENT-1)))&~((size_t)BYTE_ALIGNMENT_MASK))
#define BLOCK_SIZE_MIN	       ((size_t)(BLOCK_STRUCT_SIZE << 1))

static uint8_t heap[MM_HEAP_SIZE] __attribute__((aligned(4)));

#if USE_MM == MM_MODULE_STATIC
    static size_t mm_used = 0;

#elif USE_MM == MM_MODULE_DYNAMIC_ADDR
    static struct block_s start, *end = NULL;
    static size_t remaining = 0U;

#endif


void mm_init(void)
{
    size_t addr;
    struct block_s* first;

    start.next = (void*)heap;
    start.size = (size_t)0;

    addr = ((size_t)heap) + MM_HEAP_SIZE;
    addr -= BLOCK_STRUCT_SIZE;
    addr &= ~((size_t)BYTE_ALIGNMENT_MASK);
    end = (void*) addr;
    end->size = 0;
    end->next = NULL;

    first = (void*)heap;
    first->size = addr - (size_t)first;
    first->next = end;

    remaining = first->size;
}

void* mm_malloc(uint32_t s)
{
    void* m = NULL;
    if(s == 0) return m;
#if USE_MM == MM_MODULE_STATIC
    if(s & BYTE_ALIGNMENT_MASK) {
        s += (MM_BYTE_ALIGNMENT - (s & BYTE_ALIGNMENT_MASK));
    }

    if(((mm_used+s) < MM_HEAP_SIZE)) {
        m = heap + mm_used;
        mm_used += s;
    }
#elif USE_MM == MM_MODULE_DYNAMIC_ADDR
    struct block_s* curr, *prev, *new;

    s += BLOCK_STRUCT_SIZE;
    if((s & BYTE_ALIGNMENT_MASK) != 0x00) {
        s += (MM_BYTE_ALIGNMENT - (s & BYTE_ALIGNMENT_MASK));
    }

    if((s > 0) && (s <= remaining)) {
        prev = &start;
        curr = start.next;

        while((curr->size < s) && (curr->next != NULL)) {
            prev = curr;
            curr = curr->next;
        }

        if(curr != end) {
            m = (void*)(((uint8_t*)prev->next)+BLOCK_STRUCT_SIZE);
            prev->next=curr->next;
            curr->next = NULL;

            if((curr->size-s) > BLOCK_SIZE_MIN) {
                new = (void*)(((uint8_t*)curr)+s);
                new->size = curr->size - s;

                curr->size = s;

                // PRINT("malloc: addr:%p next:%p size:%d\n", curr, curr->next, curr->size);

                new->next=prev->next;
                prev->next=new;
            }
            remaining -= curr->size;
        }
    }
#elif MM_USE == MM_MODULE_DYNAMIC_SIZE
#else
#error "must select one mm module"
#endif

    return m;
}

void mm_free(void* m)
{
#if USE_MM == MM_MODULE_STATIC
#elif USE_MM == MM_MODULE_DYNAMIC_ADDR
    struct block_s* block;
    struct block_s* iterator;

    if(m != NULL) {
        block = (void*)((uint8_t*)m-BLOCK_STRUCT_SIZE);

        // PRINT("free: addr:%p next:%p size:%d\n", block, block->next, block->size);

        if(block->next == NULL) {
            remaining += block->size;

            for(iterator=&start; iterator->next<block; iterator=iterator->next);

            if(((uint8_t*)iterator + iterator->size) == (uint8_t*)block) {
                iterator->size += block->size;
                block = iterator;
            }
            else {
                block->next=iterator->next;
                iterator->next=block;
            }

            if(((uint8_t*)block + block->size) == (uint8_t*)(block->next)) {
                if((uint8_t*)block->next != (uint8_t*)end) {
                    block->size += block->next->size;
                    block->next = block->next->next;
                    block->next->next = NULL;
                }
            }
        }
    }
#elif MM_USE == MM_MODULE_DYNAMIC_SIZE
#else
#error "must select one mm module"
#endif
}

void mm_print_info(void)
{
    struct block_s* block;
    uint8_t i;
    PRINT("---------mm info----------\n");
    for(i=0, block=start.next; block->next!=NULL; block=block->next, i++) {
        PRINT("b%d: addr:%p next:%p size:%d\n", i, block, block->next, block->size);
    }
    PRINT("--------------------------\n");
}

