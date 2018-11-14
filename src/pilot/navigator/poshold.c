#include "commander.h"
#include "att_control.h"
#include "alt_control.h"
#include "pos_control.h"
#include "mixer.h"


void poshold_update(float dt)
{
    att_control_yaw_rate_update(dt, commander_get_yaw_rate());
    pos_control_update(dt, commander_get_roll(), commander_get_pitch());
    alt_control_update(dt, commander_get_thrust());   
}
