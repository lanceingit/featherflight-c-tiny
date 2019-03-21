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

#define USE_MODIFY_FIFO         0

void fifo_f_create(fifo_f_s *self, float *buf, uint16_t size)
{
    self->head = 0;
    self->tail = 0;
    self->data = buf;
    self->size = size;
    self->cnt = 0;
}

int8_t fifo_f_write(fifo_f_s *self, float c)
{
    if(self->cnt == self->size)
    {
        return -1;
    }
   
    self->data[self->head] = c;
    self->head++;
    self->cnt++;
    if(self->cnt > self->size) {
        self->cnt = self->size;
    }    
    if(self->head >= self->size)
    {
        self->head = 0;
    }
    
    return 0;
}

void fifo_f_write_force(fifo_f_s *self, float c)
{  
    self->data[self->head] = c;
    self->head++;
    self->cnt++;
    if(self->cnt > self->size) {
        self->cnt = self->size;
    }
    if(self->head >= self->size)
    {
        self->head = 0;
    } 
    if(self->cnt == self->size)
    {
        self->tail = self->head;
    }
}

int8_t fifo_f_read(fifo_f_s *self, float* c)
{
    if(self->cnt == 0)
    {
        return -1;
    }

    *c = self->data[self->tail];
    self->tail++;
    self->cnt--;
    if(self->tail >= self->size)
    {
        self->tail = 0;
    }

    return 0;
}

bool fifo_f_is_empty(fifo_f_s *self)
{
    return (self->head == self->tail);
}

uint16_t fifo_f_get_count(fifo_f_s *self)
{
	return self->cnt;
}

/**********************/

void fifo_create(fifo_s *self, uint8_t *buf, uint16_t size)
{
    self->head = 0;
    self->tail = 0;
    self->data = buf;
    self->size = size;
    self->cnt = 0;
}

int8_t fifo_write(fifo_s *self, uint8_t c)
{
    // PRINT("fifo_write");
    // fifo_print(fifo);

    if(self->cnt == self->size)
    {
        return -1;
    }
   
    self->data[self->head] = c;
    self->head++;
    self->cnt++;
    if(self->cnt > self->size) {
        self->cnt = self->size;
    }
    if(self->head >= self->size)
    {
        self->head = 0;
    }
    
    return 0;
}

void fifo_write_force(fifo_s *self, uint8_t c)
{  
    self->data[self->head] = c;
    self->head++;
    self->cnt++;
    if(self->cnt > self->size) {
        self->cnt = self->size;
    }
    if(self->head >= self->size)
    {
        self->head = 0;
    } 
    if(self->cnt == self->size)
    {
        self->tail = self->head;
    }

}

int8_t fifo_read(fifo_s *self, uint8_t* c)
{
    // PRINT("fifo_read");
    // fifo_print(fifo);

    if(self->cnt == 0)
    {
        // self->cnt = 0;
        return -1;
    }

    *c = self->data[self->tail];
    self->tail++;
    self->cnt--;
    if(self->tail >= self->size)
    {
        self->tail = 0;
    }

    return 0;
}

bool fifo_is_empty(fifo_s *self)
{
    return (self->head == self->tail);
}

uint16_t fifo_get_count(fifo_s *self)
{
	return self->cnt;
}

#if USE_MODIFY_FIFO
uint16_t fifo_get_tail_index(fifo_s *self)
{
    return self->tail;
}

void fifo_set_tail_index(fifo_s *self, uint16_t new_index)
{
    self->tail = new_index;
}

uint8_t* fifo_get_tail(fifo_s *self)
{
    return self->data+self->tail;
}


#define IS_TAIL_BEHAND_HEAD (self->tail < self->head)
#define IS_TAIL_FRONT_HEAD  (self->tail > self->head)
#define IS_BEYOND_HEAD(x)   (((x)>self->head && (x)<self->tail && IS_TAIL_FRONT_HEAD) \
                          || ((x)>self->head && (x)>self->tail && IS_TAIL_BEHAND_HEAD))

void fifo_set_tail(fifo_s *self, uint8_t* new_tail)
{
    uint16_t new_index;

    new_index = new_tail - self->data;
    if(new_index > self->size)
    {
        new_index -= (self->size-1);
    }

    if(IS_TAIL_BEHAND_HEAD)
    {
        if(new_index > self->head)
        {
            self->tail = self->head;
        }
        else
        {
            self->tail = new_index;
        }
    }
    else if(IS_TAIL_FRONT_HEAD)
    {
        if(IS_BEYOND_HEAD(new_index))
        {
            self->tail = self->head;
        }
        else
        {
            self->tail = new_index;
        }
    }
    else //end to end
    {
        self->tail = self->head;
    }
}

#endif //USE_MODIFY_FIFO

void fifo_print(fifo_s *self)
{
    PRINT("-------fifo--------\n");
    PRINT("addr:%p\n", self);
    PRINT("head:%u\n", self->head);
    PRINT("tail:%u\n", self->tail);
    PRINT("data:");
    for(uint8_t i=0; i<10; i++) {
        PRINT("%x ", self->data[i]);
    }
    PRINT("\n");
    PRINT("size:%u\n", self->size);
    PRINT("cnt:%u\n", self->cnt);
    PRINT("-------------------\n");
}
