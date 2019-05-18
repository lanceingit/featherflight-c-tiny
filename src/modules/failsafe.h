#pragma once

#include "timer.h"

typedef struct {
    float* low_power_threshold;
    bool is_low_power;
    float* overturn_threshold;
    bool is_overturn;
    float* link_timeout;
    bool is_link_lose;
    times_t link_time;
} Failsafe;

extern Failsafe failsafe;

void failsafe_init(void);
void failsafe_update(void);
void failsafe_link_keep(void);
        
