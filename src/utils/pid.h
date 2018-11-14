#pragma once

struct pid_s
{
	float kp;           
	float ki;           
	float kd;  
	float p_out;           
	float i_out;           
	float d_out;  
	float output;        
    float error;
    float error_prev;
    float i_limit;
    float out_limit;
    float d_weight;
};

void pid_init(struct pid_s* pid, float p,float i,float d, float i_limit, float out_limit, float d_weight);
void pid_reset(struct pid_s* pid);
float pid_update(struct pid_s* pid, float error, float dt);
