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


struct fifo_s
{
    uint16_t head;
    uint16_t tail;
    uint8_t *data;
    uint16_t size;
    uint16_t cnt;
};

struct fifo_f_s
{
    uint16_t head;
    uint16_t tail;
    float *data;
    uint16_t size;
    uint16_t cnt;
};

void fifo_create(struct fifo_s *fifo, uint8_t *buf, uint16_t size);

int8_t fifo_write(struct fifo_s *fifo, uint8_t c);
void fifo_write_force(struct fifo_s *fifo, uint8_t c);
int8_t fifo_read(struct fifo_s *fifo, uint8_t* c);

bool fifo_is_empty(struct fifo_s *fifo);
uint16_t fifo_get_count(struct fifo_s *fifo);

uint8_t* fifo_get_tail(struct fifo_s *fifo);
void fifo_set_tail(struct fifo_s *fifo, uint8_t* new_tail);

uint16_t fifo_get_tail_index(struct fifo_s *fifo);
void fifo_set_tail_index(struct fifo_s *fifo, uint16_t new_index);
void fifo_print(struct fifo_s *fifo);

void fifo_f_create(struct fifo_f_s *fifo, float *buf, uint16_t size);
int8_t fifo_f_write(struct fifo_f_s *fifo, float c);
void fifo_f_write_force(struct fifo_f_s *fifo, float c);
int8_t fifo_f_read(struct fifo_f_s *fifo, float* c);
bool fifo_f_is_empty(struct fifo_f_s *fifo);
uint16_t fifo_f_get_count(struct fifo_f_s *fifo);

#ifdef __cplusplus
}
#endif

