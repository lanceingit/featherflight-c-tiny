#pragma once

#include "est.h"

struct pos_est_inav_s
{
	struct pos_est_s heir;    
};

extern struct pos_est_inav_s pos_est_inav;

bool pos_est_inav_init(void);
bool pos_est_inav_run(float dt);
