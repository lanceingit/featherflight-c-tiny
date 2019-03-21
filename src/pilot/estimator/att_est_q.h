#pragma once

#include "est.h"
#include "lpf.h"

struct att_est_q_s
{
	struct att_est_s heir;
    
	bool mag_decl_auto;
	float mag_decl;
    Vector mag_earth;
	float bias_max;
	float w_accel;
	float w_mag;
	float w_gyro_bias;    

    Vector rate;

	lpf2p_s	acc_filter_x;
	lpf2p_s	acc_filter_y;
	lpf2p_s	acc_filter_z;
	lpf2p_s	gyro_filter_x;
	lpf2p_s	gyro_filter_y;
	lpf2p_s	gyro_filter_z;
};

extern struct att_est_q_s att_est_q;

bool att_est_q_init(void);
bool att_est_q_run(float dt);
