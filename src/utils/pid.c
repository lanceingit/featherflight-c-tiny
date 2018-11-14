#include "pid.h"
#include "mathlib.h"

void pid_init(struct pid_s* pid, float p,float i,float d, float i_limit, float out_limit, float d_weight)
{
    pid->kp = p; 
    pid->ki = i; 
    pid->kd = d; 
    pid->i_limit = i_limit;
    pid->out_limit = out_limit;
    pid->d_weight = d_weight;
	pid->error_prev = 0.0f;
	pid->output = 0.0f;
	pid->i_out = 0.0f;
	pid->d_out = 0.0f;    
}

void pid_reset(struct pid_s* pid)
{
	pid->error_prev = 0.0f;
	pid->output = 0.0f;
	pid->i_out = 0.0f;
	pid->d_out = 0.0f;    
}

float pid_update(struct pid_s* pid, float error, float dt)
{
    float output = 0.0f;

    pid->error = error;

    pid->p_out = pid->kp * pid->error;

    pid->i_out += pid->ki * pid->error * dt;
    pid->i_out = constrain(pid->i_out, -pid->i_limit, pid->i_limit);    

    float d = (pid->error - pid->error_prev) / dt;
    pid->d_out = (d * pid->d_weight + (1.0f - pid->d_weight) * pid->d_out)*pid->kd;

    output = pid->p_out + pid->i_out + pid->d_out;
    output = constrain(output, -pid->out_limit, pid->out_limit);

    pid->error_prev = pid->error;

    return output;
}
