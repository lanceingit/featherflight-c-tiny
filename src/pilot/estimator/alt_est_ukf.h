#pragma once

#include "est.h"
#include "srcdkf.h"

struct alt_est_ukf_s
{
    struct alt_est_s heir;
    struct srcdkf_s *kf;
    float *x;    

    Vector acc_ned;
    Vector acc_body;
    Vector acc_bias;
   
};

extern struct alt_est_ukf_s alt_est_ukf;

bool alt_est_ukf_init(void);
bool alt_est_ukf_run(float dt);
