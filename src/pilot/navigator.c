#include "board.h"
#include "navigator.h"
#include "timer.h"

typedef void(*nav_update_func)(float dt);

struct navigator_s
{
    enum nav_mode curr_mode;
    times_t last_update_time;
    nav_update_func nav_update;
};

struct navigator_s navigator = {
    .curr_mode = NAV_STOP,
    .nav_update = NULL,
};

static struct navigator_s* this = &navigator;

bool navigator_set_mode(enum nav_mode mode)
{
    bool ret = false;

    if (mode == this->curr_mode) {
        return true;
    }

    switch(mode) {
        case NAV_STABILIZE:
            if((ret=stabilize_init())) {
                this->nav_update = stabilize_update;
            }
            break;
        case NAV_ALTHOLD:
            if((ret=althold_init())) {
                this->nav_update = althold_update;
            }
            break;
        case NAV_POSHOLD:
            break;
        case NAV_TAKEOFF:
            break;
        case NAV_LAND:      
            break;
        case NAV_STOP:        
            break;
        default:break;
    }

    if (ret == true) {
        this->curr_mode = mode;
    }

    return ret;    
}

enum nav_mode navigator_get_mode(void)
{
    return this->curr_mode;
}

void navigator_update(void)
{
    float dt = timer_get_dt(&this->last_update_time, 0.02f, 0.00001f);
    if(this->nav_update != NULL) {
        this->nav_update(dt);
    }
}

