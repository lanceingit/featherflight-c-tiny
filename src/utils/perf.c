#include "board.h"

#include "perf.h"
#include "timer.h"
#include "debug.h"

void perf_init(struct perf_s* perf)
{
    perf->t0=0;
    perf->t1 = 0;
    perf->cnt=0;
    perf->avg=0;
    perf->sum=0;
    perf->t_max=0;
    perf->t_min=0xFFFFFFFF;
}

void perf_interval(struct perf_s* perf)
{
    perf->t1 = timer_now() - perf->t0;
    if(perf->t0 != 0)
    {
        perf->sum += perf->t1;
        perf->cnt++;
        perf->avg = perf->sum / perf->cnt;
        if(perf->t1>perf->t_max) perf->t_max=perf->t1;
        if(perf->t1<perf->t_min) perf->t_min=perf->t1;
    }
    perf->t0 = timer_now();
}

void perf_begin(struct perf_s* perf)
{
    perf->t0 = timer_now();
}

void perf_end(struct perf_s* perf)
{
    perf->t1 = timer_now() - perf->t0;
    if(perf->t0 != 0)
    {
        perf->sum += perf->t1;
        perf->cnt++;
        perf->avg = perf->sum / perf->cnt;
        if(perf->t1>perf->t_max) perf->t_max=perf->t1;
        if(perf->t1<perf->t_min) perf->t_min=perf->t1;
    }
}

void perf_print(struct perf_s* perf, char* name)
{
    PRINT("%s:curr:%06lld avg:%lld min:%lld max:%lld\n", name, perf->t1, perf->avg, perf->t_min, perf->t_max);
}
