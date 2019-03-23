#pragma once

#include "est.h"

typedef struct
{
    AttEst heir;

    float dcm[3][3];

    float kp;
    float ki;
} AttEstCf;

extern AttEstCf att_est_cf;

bool att_est_cf_init(void);
bool att_est_cf_run(float dt);
