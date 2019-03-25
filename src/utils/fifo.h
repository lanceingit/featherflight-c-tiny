/**
 *
 * say something about project
 *
 * fifo.h
 *
 * v1.0
 *
 * say something about file
 */


#pragma once

#ifdef __cplusplus
 extern "C" {
#endif 
     
#include <stdint.h>
#include <stdbool.h>


typedef struct 
{
    uint16_t head;
    uint16_t tail;
    uint8_t *data;
    uint16_t size;
    uint16_t cnt;
} Fifo;

typedef struct 
{
    uint16_t head;
    uint16_t tail;
    float *data;
    uint16_t size;
    uint16_t cnt;
} FifoF;

void fifo_init(Fifo *self, uint8_t *buf, uint16_t size);
int8_t fifo_write(Fifo *self, uint8_t c);
void fifo_write_force(Fifo *self, uint8_t c);
int8_t fifo_read(Fifo *self, uint8_t* c);

bool fifo_is_empty(Fifo *self);
uint16_t fifo_get_count(Fifo *self);

#if USE_MODIFY_FIFO
uint8_t* fifo_get_tail(Fifo *self);
void fifo_set_tail(Fifo *self, uint8_t* new_tail);

uint16_t fifo_get_tail_index(Fifo *self);
void fifo_set_tail_index(Fifo *self, uint16_t new_index);
#endif //USE_MODIFY_FIFO

void fifo_print(Fifo *self);

/**********************/

void fifo_f_init(FifoF *self, float *buf, uint16_t size);
int8_t fifo_f_write(FifoF *self, float c);
void fifo_f_write_force(FifoF *self, float c);
int8_t fifo_f_read(FifoF *self, float* c);
bool fifo_f_is_empty(FifoF *self);
uint16_t fifo_f_get_count(FifoF *self);

#ifdef __cplusplus
}
#endif

