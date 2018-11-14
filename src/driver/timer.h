/**
 *
 * timer.h
 *
 * simple timer, delay and time block function 
 */


#pragma once

typedef uint64_t times_t;

#define TIMER_DEF(name) static times_t name = 0;

void timer_init(void);
void timer_disable(void);

times_t timer_new(uint32_t us);
bool timer_is_timeout(times_t* t);

times_t timer_now(void);
times_t timer_elapsed(times_t* t);

bool timer_check(times_t* t, uint32_t us);
float timer_get_dt(times_t* t, float max, float min);
    
void delay(float s);
void delay_ms(uint32_t ms);
void delay_us(uint32_t us);
#ifdef F3_EVO
void sleep(float s);
#endif





     

