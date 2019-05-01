/**                                               _____           ,-.
 * _______       _____       _____                ___   _,.      /  /
 * ___    |__   ____(_)_____ __  /______________  __   ; \____,-==-._  )
 * __  /| |_ | / /_  /_  __ `/  __/  __ \_  ___/  _    //_    `----' {+>
 * _  ___ |_ |/ /_  / / /_/ // /_ / /_/ /  /      _    `  `'--/  /-'`(
 * /_/  |_|____/ /_/  \__,_/ \__/ \____//_/       _          /  /
 *                                                           `='
 *
 * timer.c
 *
 * v2.0
 *
 * simple timer, delay and time block function
 */

#include "board.h"

#include "timer.h"
#ifdef LINUX
    #include <time.h>
#endif

#if USE_RTC
    #include "rtc.h"
#endif


static volatile times_t timer_cnt = 0;
#ifdef LINUX
    static struct timespec boot_time;
#elif ESP
    static os_timer_t work_timer;
#endif


times_t timer_now(void)
{
#ifdef LINUX
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    return (times_t)((now.tv_sec-boot_time.tv_sec)*1000000+(now.tv_nsec-boot_time.tv_nsec) / 1000);
#else
    return timer_cnt*US_PER_TICK;
#endif
}

times_t timer_new(uint32_t us)
{
    return timer_now()+us;
}

bool timer_is_timeout(times_t* t)
{
    if(*t >= timer_now()) {
        return false;
    }
    else {
        return true;
    }
}

static times_t timer_passed(times_t* since)
{
    times_t now = timer_now();

    if(now >= *since) {
        return now-*since;
    }

    return now+(TIME_MAX-*since+1);
}

times_t timer_elapsed(times_t* t)
{
    return timer_passed(t);
}

bool timer_check(times_t* t, times_t us)
{
    if(timer_passed(t) > us) {
        *t = timer_now();
        return true;
    }
    return false;
}

float timer_get_dt(times_t* t, float max, float min)
{
    float dt = (*t > 0) ? ((timer_now()-*t) / 1000000.0f) : min;
    *t = timer_now();

    if(dt > max) {
        dt = max;
    }
    if(dt < min) {
        dt = min;
    }
    return dt;
}

void delay(float s)
{
    volatile times_t wait;

    wait = timer_new((uint32_t)(s*1000*1000));
    while(!timer_is_timeout((times_t*)&wait));
}

void delay_ms(uint32_t ms)
{
    volatile times_t wait;

    wait = timer_new(ms*1000);
    while(!timer_is_timeout((times_t*)&wait));
}

void delay_us(uint32_t us)
{
    volatile times_t wait;

    wait = timer_new(us);
    while(!timer_is_timeout((times_t*)&wait));
}

#ifndef LINUX
void sleep(float s)
{
    volatile times_t wait;

    wait = timer_new((uint32_t)(s*1000*1000));
    while(!timer_is_timeout((times_t*)&wait));
}
#endif

void timer_disable(void)
{
#ifdef STM32F3
    NVIC_DisableIRQ(TIM7_IRQn);

#elif ESP
    os_timer_disarm(&work_timer);

#elif SM701
    am_hal_ctimer_stop(2, AM_HAL_CTIMER_TIMERA);
#endif
}

#ifdef STM32F3
static void timer_irs(void)
{
    TIM_ClearFlag(TIM7, TIM_IT_Update);
    timer_cnt++;
}

#elif ESP
void timer_irs(void* arvg)
{
    timer_cnt++;

#if USE_RTC
    if(timer_cnt%(1000*1000/US_PER_TICK) == 0) {
        rtc_second_inc();
    }
#endif
}

#elif SM701
static void timer_irs(void)
{
    uint32_t status;

    status = am_hal_ctimer_int_status_get(true);

    am_hal_ctimer_int_clear(status);

    if(status & AM_HAL_CTIMER_INT_TIMERA2) {
        timer_cnt++;
    }
}
#endif

#ifdef STM32F3
void TIM7_IRQHandler(void)
{
    timer_irs();
}

#elif SM701
void am_ctimer_isr(void)
{
    timer_irs();
}

#endif

void timer_init()
{
#ifdef STM32F3
    TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    TIM_DeInit(TIM7);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Period = 10-1;                //10us
    TIM_TimeBaseStructure.TIM_Prescaler = 72;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);

    NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_ARRPreloadConfig(TIM7, DISABLE);

    TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);

    TIM_Cmd(TIM7, ENABLE);
#elif LINUX
    clock_gettime(CLOCK_MONOTONIC, &boot_time);

#elif ESP
    os_timer_setfn(&work_timer, timer_irs, NULL);
    os_timer_arm(&work_timer, US_PER_TICK/1000, 1);

#elif SM701
    am_hal_ctimer_config_single(2, AM_HAL_CTIMER_TIMERA,
                                AM_HAL_CTIMER_XT_2_048KHZ |
                                AM_HAL_CTIMER_FN_REPEAT |
                                AM_HAL_CTIMER_INT_ENABLE
                               );

    am_hal_ctimer_period_set(2, AM_HAL_CTIMER_TIMERA, (2048/TICK_RATE_MHZ - 1), 0);   //1.953125us

    am_hal_ctimer_int_enable(AM_HAL_CTIMER_INT_TIMERA2);
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_CTIMER);

    am_hal_ctimer_start(2, AM_HAL_CTIMER_TIMERA);
#endif
}
