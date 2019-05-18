#pragma once

typedef enum
{
    NAV_STABILIZE = 0,
    NAV_ALTHOLD,
    NAV_POSHOLD,
    NAV_TAKEOFF,
    NAV_LAND,      
    NAV_STOP,
} nav_mode_e;

void navigator_update(void);

bool navigator_set_mode(nav_mode_e mode);
nav_mode_e navigator_get_mode(void);

#include "stabilize.h"
#include "althold.h"

