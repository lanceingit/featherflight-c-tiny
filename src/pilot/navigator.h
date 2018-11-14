#pragma once

enum nav_mode
{
    NAV_STABILIZE = 0,
    NAV_ALTHOLD,
    NAV_POSHOLD,
    NAV_TAKEOFF,
    NAV_LAND,      
    NAV_STOP,
};

void navigator_update(void);

bool navigator_set_mode(enum nav_mode mode);
enum nav_mode navigator_get_mode(void);

#include "stabilize.h"
#include "althold.h"

