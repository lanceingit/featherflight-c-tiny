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
} fifo_s;

typedef struct 
{
    uint16_t head;
    uint16_t tail;
    float *data;
    uint16_t size;
    uint16_t cnt;
} fifo_f_s;

void fifo_create(fifo_s *self, uint8_t *buf, uint16_t size);

int8_t fifo_write(fifo_s *self, uint8_t c);
void fifo_write_force(fifo_s *self, uint8_t c);
int8_t fifo_read(fifo_s *self, uint8_t* c);

bool fifo_is_empty(fifo_s *self);
uint16_t fifo_get_count(fifo_s *self);

uint8_t* fifo_get_tail(fifo_s *self);
void fifo_set_tail(fifo_s *self, uint8_t* new_tail);

uint16_t fifo_get_tail_index(fifo_s *self);
void fifo_set_tail_index(fifo_s *self, uint16_t new_index);
void fifo_print(fifo_s *self);

void fifo_f_create(fifo_f_s *self, float *buf, uint16_t size);
int8_t fifo_f_write(fifo_f_s *self, float c);
void fifo_f_write_force(fifo_f_s *self, float c);
int8_t fifo_f_read(fifo_f_s *self, float* c);
bool fifo_f_is_empty(fifo_f_s *self);
uint16_t fifo_f_get_count(fifo_f_s *self);

#ifdef __cplusplus
}
#endif

