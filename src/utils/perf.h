#pragma once

#include "timer.h"

#define PERF_DEF(name) static struct perf_s name = { \
    .t0 = 0, \
    .t1 = 0, \
    .cnt = 0, \
    .avg = 0, \
    .sum = 0, \
    .t_max = 0, \
    .t_min = 0xFFFFFFFF, \
};

struct perf_s
{
    times_t t0;
    times_t t1;
    uint64_t cnt;
    times_t avg;
    times_t sum;
    times_t t_max;
    times_t t_min;
};

void perf_init(struct perf_s* perf);
void perf_interval(struct perf_s* perf);
void perf_begin(struct perf_s* perf);
void perf_end(struct perf_s* perf);
void perf_print(struct perf_s* perf, char* name);

