#include "board.h"

#include "perf.h"
#include "timer.h"
#include "debug.h"

void perf_init(Perf* self)
{
    self->t0=0;
    self->t1 = 0;
    self->cnt=0;
    self->avg=0;
    self->sum=0;
    self->t_max=0;
    self->t_min=0xFFFFFFFF;
}

void perf_interval(Perf* self)
{
    self->t1 = timer_now() - self->t0;
    if(self->t0 != 0)
    {
        self->sum += self->t1;
        self->cnt++;
        self->avg = self->sum / self->cnt;
        if(self->t1>self->t_max) self->t_max=self->t1;
        if(self->t1<self->t_min) self->t_min=self->t1;
    }
    self->t0 = timer_now();
}

void perf_begin(Perf* self)
{
    self->t0 = timer_now();
}

void perf_end(Perf* self)
{
    self->t1 = timer_now() - self->t0;
    if(self->t0 != 0)
    {
        self->sum += self->t1;
        self->cnt++;
        self->avg = self->sum / self->cnt;
        if(self->t1>self->t_max) self->t_max=self->t1;
        if(self->t1<self->t_min) self->t_min=self->t1;
    }
}

void perf_print(Perf* self, char* name)
{
    PRINT("%s:curr:%06lld avg:%lld min:%lld max:%lld\n", name, self->t1, self->avg, self->t_min, self->t_max);
}
