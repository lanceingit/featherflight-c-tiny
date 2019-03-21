#include "pid.h"
#include "est.h"
#include "mixer.h"
#include "mathlib.h"
#include "param.h"
#include "attc_param.h"


pid_s att_pid_roll;
pid_s att_pid_pitch;
pid_s att_pid_yaw;

pid_s rate_pid_roll;
pid_s rate_pid_pitch;
pid_s rate_pid_yaw;


void att_control_init(void)
{
    PARAM_REGISTER(attc)
	pid_init(&att_pid_roll, PARAM_GET(ATTC_ATT_ROLL_P), 0, 0, 0, PARAM_GET(ATTC_ATT_RP_OUT_LIMIT), 0);
	pid_init(&att_pid_pitch, PARAM_GET(ATTC_ATT_PITCH_P), 0, 0, 0, PARAM_GET(ATTC_ATT_RP_OUT_LIMIT), 0);
	pid_init(&att_pid_yaw, PARAM_GET(ATTC_ATT_YAW_P), 0, 0, 0, PARAM_GET(ATTC_ATT_YAW_OUT_LIMIT), 0);
	
	pid_init(&rate_pid_roll, PARAM_GET(ATTC_RATE_ROLL_P),
                                PARAM_GET(ATTC_RATE_ROLL_I),
                                PARAM_GET(ATTC_RATE_ROLL_D),
                                PARAM_GET(ATTC_RATE_RP_I_LIMIT),
                                PARAM_GET(ATTC_RATE_RP_OUT_LIMIT),
                                PARAM_GET(ATTC_RATE_RP_D_WEIGHT));
	pid_init(&rate_pid_pitch, PARAM_GET(ATTC_RATE_PITCH_P),
                                PARAM_GET(ATTC_RATE_PITCH_I),
                                PARAM_GET(ATTC_RATE_PITCH_D),
                                PARAM_GET(ATTC_RATE_RP_I_LIMIT),
                                PARAM_GET(ATTC_RATE_RP_OUT_LIMIT),
                                PARAM_GET(ATTC_RATE_RP_D_WEIGHT));
	pid_init(&rate_pid_yaw, PARAM_GET(ATTC_RATE_YAW_P),
                                PARAM_GET(ATTC_RATE_YAW_I),
                                PARAM_GET(ATTC_RATE_YAW_D),
                                PARAM_GET(ATTC_RATE_YAW_I_LIMIT),
                                PARAM_GET(ATTC_RATE_YAW_OUT_LIMIT),
                                PARAM_GET(ATTC_RATE_YAW_D_WEIGHT));                                                                
}

void att_control_roll_pitch_rate_update(float dt, float roll_rate_target, float pitch_rate_target, float limit)
{
	float roll_output;
	float pitch_output;  

	roll_rate_target = constrain(roll_rate_target, -limit, limit);
	pitch_rate_target = constrain(pitch_rate_target, -limit, limit);

	roll_output = pid_update(&rate_pid_roll, roll_rate_target-att->roll_rate, dt);
	pitch_output = pid_update(&rate_pid_pitch, pitch_rate_target-att->pitch_rate, dt);

	mixer_set_roll(roll_output);
	mixer_set_pitch(pitch_output);       
}

void att_control_roll_pitch_update(float dt, float roll_target, float pitch_target, float limit)
{
	float rate_taret_roll;
	float rate_taret_pitch;    

	roll_target = constrain(roll_target, -PARAM_GET(ATTC_RP_LIMIT), PARAM_GET(ATTC_RP_LIMIT));
	pitch_target = constrain(pitch_target, -PARAM_GET(ATTC_RP_LIMIT), PARAM_GET(ATTC_RP_LIMIT));

    rate_taret_roll  = pid_update(&att_pid_roll, roll_target-att->roll, dt);
    rate_taret_pitch = pid_update(&att_pid_pitch, pitch_target-att->pitch, dt);

    att_control_roll_pitch_rate_update(dt, rate_taret_roll, rate_taret_pitch, limit);
}

void att_control_yaw_rate_update(float dt, float yaw_rate_target)
{
    float yaw_output;  

    yaw_output = pid_update(&rate_pid_yaw, yaw_rate_target-att->yaw_rate, dt);
    mixer_set_yaw(yaw_output);  
}

void att_control_yaw_update(float dt, float yaw_target)
{
	float rate_taret_yaw;

    rate_taret_yaw   = pid_update(&att_pid_yaw, yaw_target-att->yaw, dt);
    att_control_yaw_rate_update(dt, rate_taret_yaw);
}

