#include "board.h"
#include "debug.h"
#include "commander.h"
#include "att_control.h"
#include "mixer.h"

bool stabilize_init(void)
{
    PRINT("stabilize init!\n");
    return true;
}

void stabilize_update(float dt)
{
//    PRINT("r:%f p:%f y:%f t:%f\n", commander_get_roll(), commander_get_pitch(), commander_get_yaw_rate(), commander_get_thrust());
    att_control_roll_pitch_update(dt, commander_get_roll(), commander_get_pitch(), 1.0f);
    att_control_yaw_rate_update(dt, commander_get_yaw_rate());
    mixer_set_thrust(commander_get_thrust());
}
