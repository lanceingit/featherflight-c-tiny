#pragma once

#include "est.h"

struct att_est_cf_s
{
    struct att_est_s heir;

    float dcm[3][3];

    float kp;
    float ki;
};

extern struct att_est_cf_s att_est_cf;

bool att_est_cf_init(void);
bool att_est_cf_run(float dt);
