#pragma once

#include "timer.h"

#define PERF_DEF(name) static Perf name = { \
    .t0 = 0, \
    .t1 = 0, \
    .cnt = 0, \
    .avg = 0, \
    .sum = 0, \
    .t_max = 0, \
    .t_min = 0xFFFFFFFF, \
};

typedef struct
{
    times_t t0;
    times_t t1;
    uint64_t cnt;
    times_t avg;
    times_t sum;
    times_t t_max;
    times_t t_min;
} Perf;

void perf_init(Perf* self);
void perf_interval(Perf* self);
void perf_begin(Perf* self);
void perf_end(Perf* self);
void perf_print(Perf* self, char* name);

