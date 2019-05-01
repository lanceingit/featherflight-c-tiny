#pragma once

#if TIMER_WIDTH_BIT==64
    typedef uint64_t times_t;
    #define TIME_MAX 0xFFFFFFFFFFFFFFFF
#elif TIMER_WIDTH_BIT==32
    typedef uint32_t times_t;
    #define TIME_MAX 0xFFFFFFFF
#else
    #error "timer width too low or not align"
#endif

#define TIMER_DEF(name) static times_t name = 0;

void timer_init(void);
void timer_disable(void);

times_t timer_new(uint32_t us);
bool timer_is_timeout(times_t* t);

times_t timer_now(void);
times_t timer_elapsed(times_t* t);

bool timer_check(times_t* t, times_t us);
float timer_get_dt(times_t* t, float max, float min);

void delay(float s);
void delay_ms(uint32_t ms);
void delay_us(uint32_t us);
#ifndef LINUX
    void sleep(float s);
#endif







