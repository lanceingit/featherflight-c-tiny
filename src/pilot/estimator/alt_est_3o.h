#pragma once

#include "est.h"

struct alt_est_3o_s
{
	struct alt_est_s heir;    
    float k1;
    float k2;
    float k3;
    float alt_corr;
    float vel_corr;
    float acc_corr;
    float alt_err;
    float pos_predict;
};

extern struct alt_est_3o_s alt_est_3o;

bool alt_est_3o_init(void);
bool alt_est_3o_run(float dt);
