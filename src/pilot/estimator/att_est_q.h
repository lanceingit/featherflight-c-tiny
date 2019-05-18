#pragma once

#include "est.h"
#include "lpf.h"

typedef struct
{
	AttEst heir;
    
	bool mag_decl_auto;
	float mag_decl;
    Vector mag_earth;
	float* bias_max;
	float* w_accel;
	float* w_mag;
	float* w_gyro_bias;    

    Vector rate;

	Lpf2p	acc_filter_x;
	Lpf2p	acc_filter_y;
	Lpf2p	acc_filter_z;
	Lpf2p	gyro_filter_x;
	Lpf2p	gyro_filter_y;
	Lpf2p	gyro_filter_z;
} AttEstQ;

extern AttEstQ att_est_q;

bool att_est_q_init(void);
bool att_est_q_run(float dt);
