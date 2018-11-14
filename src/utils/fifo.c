/**
 *
 * say something about project
 *
 * fifo.c
 *
 * v1.0
 *
 * say something about file
 */
#include "board.h"

#include "fifo.h"
#include "debug.h"



void fifo_f_create(struct fifo_f_s *fifo, float *buf, uint16_t size)
{
    fifo->head = 0;
    fifo->tail = 0;
    fifo->data = buf;
    fifo->size = size;
    fifo->cnt = 0;
}

int8_t fifo_f_write(struct fifo_f_s *fifo, float c)
{
    if(fifo->cnt == fifo->size)
    {
        return -1;
    }
   
    fifo->data[fifo->head] = c;
    fifo->head++;
    fifo->cnt++;
    if(fifo->cnt > fifo->size) {
        fifo->cnt = fifo->size;
    }    
    if(fifo->head >= fifo->size)
    {
        fifo->head = 0;
    }
    
    return 0;
}

void fifo_f_write_force(struct fifo_f_s *fifo, float c)
{  
    fifo->data[fifo->head] = c;
    fifo->head++;
    fifo->cnt++;
    if(fifo->cnt > fifo->size) {
        fifo->cnt = fifo->size;
    }
    if(fifo->head >= fifo->size)
    {
        fifo->head = 0;
    } 
    if(fifo->cnt == fifo->size)
    {
        fifo->tail = fifo->head;
    }
}

int8_t fifo_f_read(struct fifo_f_s *fifo, float* c)
{
    if(fifo->cnt == 0)
    {
        return -1;
    }

    *c = fifo->data[fifo->tail];
    fifo->tail++;
    fifo->cnt--;
    if(fifo->tail >= fifo->size)
    {
        fifo->tail = 0;
    }

    return 0;
}

bool fifo_f_is_empty(struct fifo_f_s *fifo)
{
    return (fifo->head == fifo->tail);
}

uint16_t fifo_f_get_count(struct fifo_f_s *fifo)
{
	return fifo->cnt;
}

/**********************/

void fifo_create(struct fifo_s *fifo, uint8_t *buf, uint16_t size)
{
    fifo->head = 0;
    fifo->tail = 0;
    fifo->data = buf;
    fifo->size = size;
    fifo->cnt = 0;
}

int8_t fifo_write(struct fifo_s *fifo, uint8_t c)
{
    // PRINT("fifo_write");
    // fifo_print(fifo);

    if(fifo->cnt == fifo->size)
    {
        return -1;
    }
   
    fifo->data[fifo->head] = c;
    fifo->head++;
    fifo->cnt++;
    if(fifo->cnt > fifo->size) {
        fifo->cnt = fifo->size;
    }
    if(fifo->head >= fifo->size)
    {
        fifo->head = 0;
    }
    
    return 0;
}

void fifo_write_force(struct fifo_s *fifo, uint8_t c)
{  
    fifo->data[fifo->head] = c;
    fifo->head++;
    fifo->cnt++;
    if(fifo->cnt > fifo->size) {
        fifo->cnt = fifo->size;
    }
    if(fifo->head >= fifo->size)
    {
        fifo->head = 0;
    } 
    if(fifo->cnt == fifo->size)
    {
        fifo->tail = fifo->head;
    }

}

int8_t fifo_read(struct fifo_s *fifo, uint8_t* c)
{
    // PRINT("fifo_read");
    // fifo_print(fifo);

    if(fifo->cnt == 0)
    {
        // fifo->cnt = 0;
        return -1;
    }

    *c = fifo->data[fifo->tail];
    fifo->tail++;
    fifo->cnt--;
    if(fifo->tail >= fifo->size)
    {
        fifo->tail = 0;
    }

    return 0;
}

bool fifo_is_empty(struct fifo_s *fifo)
{
    return (fifo->head == fifo->tail);
}

uint16_t fifo_get_count(struct fifo_s *fifo)
{
	return fifo->cnt;
}

uint16_t fifo_get_tail_index(struct fifo_s *fifo)
{
    return fifo->tail;
}

void fifo_set_tail_index(struct fifo_s *fifo, uint16_t new_index)
{
    fifo->tail = new_index;
}

uint8_t* fifo_get_tail(struct fifo_s *fifo)
{
    return fifo->data+fifo->tail;
}


#define IS_TAIL_BEHAND_HEAD (fifo->tail < fifo->head)
#define IS_TAIL_FRONT_HEAD  (fifo->tail > fifo->head)
#define IS_BEYOND_HEAD(x)   (((x)>fifo->head && (x)<fifo->tail && IS_TAIL_FRONT_HEAD) \
                          || ((x)>fifo->head && (x)>fifo->tail && IS_TAIL_BEHAND_HEAD))

void fifo_set_tail(struct fifo_s *fifo, uint8_t* new_tail)
{
    uint16_t new_index;

    new_index = new_tail - fifo->data;
    if(new_index > fifo->size)
    {
        new_index -= (fifo->size-1);
    }

    if(IS_TAIL_BEHAND_HEAD)
    {
        if(new_index > fifo->head)
        {
            fifo->tail = fifo->head;
        }
        else
        {
            fifo->tail = new_index;
        }
    }
    else if(IS_TAIL_FRONT_HEAD)
    {
        if(IS_BEYOND_HEAD(new_index))
        {
            fifo->tail = fifo->head;
        }
        else
        {
            fifo->tail = new_index;
        }
    }
    else //end to end
    {
        fifo->tail = fifo->head;
    }
}

void fifo_print(struct fifo_s *fifo)
{
    PRINT("-------fifo--------\n");
    PRINT("addr:%p\n", fifo);
    PRINT("head:%u\n", fifo->head);
    PRINT("tail:%u\n", fifo->tail);
    PRINT("data:");
    for(uint8_t i=0; i<10; i++) {
        PRINT("%x ", fifo->data[i]);
    }
    PRINT("\n");
    PRINT("size:%u\n", fifo->size);
    PRINT("cnt:%u\n", fifo->cnt);
    PRINT("-------------------\n");
}