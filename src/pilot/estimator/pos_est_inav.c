#include "board.h"

#include "pos_est_inav.h"
#include "debug.h"


struct pos_est_inav_s pos_est_inav = {
	.heir = {
        .init = &pos_est_inav_init,
        .run = &pos_est_inav_run,
	},
};

static struct pos_est_inav_s* this=&pos_est_inav;

void pos_est_inav_shell(int argc, char *argv[]);


bool pos_est_inav_init(void)
{
    cli_regist("posi", pos_est_inav_shell);
    this->heir.inited = true;

    return true;
}

bool pos_est_inav_run(float dt)
{
    if(n_flow >= 100) {
        n_flow = 0;
        gyro_smooth[0] = 0.0f;
        gyro_smooth[1] = 0.0f;
        gyro_smooth[2] = 0.0f;

    } else {
        gyro_smooth[0] = (att->pitch_rate + n_flow * gyro_smooth[0]) / (n_flow + 1);
        gyro_smooth[1] = (att->roll_rate + n_flow * gyro_smooth[1]) / (n_flow + 1);
        gyro_smooth[2] = (att->yaw_rate + n_flow * gyro_smooth[2]) / (n_flow + 1);
        n_flow++;
    }
    //TODO:
    // yaw_comp[0] = - params.flow_module_offset_y * (flow_gyrospeed[2] - gyro_offset_filtered[2]);
    // yaw_comp[1] = params.flow_module_offset_x * (flow_gyrospeed[2] - gyro_offset_filtered[2]);


    return true;
}

void pos_est_inav_shell(int argc, char *argv[])
{
    
}
