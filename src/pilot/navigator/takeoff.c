#include "board.h"
#include "pilot.h"

static uint8_t status;

bool takeoff_init(void)
{
    return true;
}

void takeoff_update(float dt)
{
    if(fabs(commander_get_pitch()) > 0 || fabs(commander_get_roll()) > 0) {
        att_control_roll_pitch_update(dt, commander_get_roll(), commander_get_pitch(), 1.0f);
    }else{
        if(pos_est_valid()) {
        }else{
            att_control_roll_pitch_update(dt, commander_get_roll(), commander_get_pitch(), 1.0f);
        }
    }

    att_control_yaw_rate_update(dt, commander_get_yaw_rate());
    alt_control_update(dt, commander_get_vel_z());

    if(fabs(commander_get_thrust()) > 0.8f || fabs(pos.pos_ned[2] - tkof_pos_sp[2]) < 0.1f){
        if(est.acc_bias_body[2] < 0.02f) {
            pos_control_set_alt_step_slow();
        } else {
            pos_control_set_alt_step_normal();
        }
        inav_baro_set_normal_mode();
        inav_flow_set_normal_mode();
        att_control_set_att_pid_normal_mode();
        pos_control_set_vel_pid_normal_mode();
        pos_control_set_alt_pid_normal_mode();
        //control_alt_set_last_vel_z_target_postive();
        pos_control_set_pos_ned_z_target(nav_get_pos_ned_z());
        control_set_normal_mode();
    }    
}
