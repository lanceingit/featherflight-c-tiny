#include "altc_param.h"
#include "pilot.h"
#include "pid.h"
#include "timer.h"
#include "lpf.h"
#include "commander.h"
#include <math.h>

struct pid_s alt_vel_pid;
struct pid_s alt_pos_pid;

static float alt_target;

void alt_control_init(void)
{
    PARAM_REGISTER(altc)
	pid_init(&alt_pos_pid, PARAM_GET(ALTC_POS_P), 0.0f, 0.0f, 0.0f, PARAM_GET(ATTC_POS_OUT_LIMIT), 0.0f);
	
	pid_init(&alt_vel_pid, PARAM_GET(ALTC_VEL_P),
                            PARAM_GET(ALTC_VEL_I),
                            PARAM_GET(ALTC_VEL_D),
                            PARAM_GET(ALTC_VEL_I_LIMIT),
                            PARAM_GET(ALTC_VEL_OUT_LIMIT),
                            PARAM_GET(ALTC_VEL_D_WEIGHT));                                                            
}

void alt_control_update(float dt, float vel_target)
{
    float thrust_output;

    if(fabsf(vel_target) > 0.0f) {
        alt_target = alt->alt;
    } else {
		if(commader_get_alt_scene() == ALT_MOVE_UP) {
			//悬停不稳时，使用速度控制。能有效抑制上拉后掉高	
			alt_target = lpfrc_apply(alt_target, alt->alt, 0.95f);
		} else if(commader_get_alt_scene() == ALT_MOVE_DOWN) {
			//悬停不稳时，使用位置控制，但对目标高度做平滑。能有效抑制下拉后回弹
			alt_target = lpfrc_apply(alt_target, alt->alt, 0.95f);
		}
		vel_target = pid_update(&alt_pos_pid, alt_target-alt->alt, dt);
    }
	thrust_output = pid_update(&alt_vel_pid, vel_target-alt->vel, dt);
    mixer_set_thrust(thrust_output);
}
