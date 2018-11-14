#pragma once

#include "est.h"

struct alt_est_inav_s
{
	struct alt_est_s heir;    
    float w_pos;
    float w_vel;
    float w_bias;
    float w_corr2bias;
    Vector acc_bias;
    Vector acc_bias_corr;
    float acc_alt;
    float acc_vel;
    float acc_vel_bias;
    float baro_offset;
    float baro_corr;
    float bias_inited;
    float w_baro_off;
    float baro_check_vel; 
};

extern struct alt_est_inav_s alt_est_inav;

bool alt_est_inav_init(void);
bool alt_est_inav_run(float dt);
