#pragma once

typedef struct
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
} pid_s;

void pid_init(pid_s* self, float p,float i,float d, float i_limit, float out_limit, float d_weight);
void pid_reset(pid_s* self);
float pid_update(pid_s* self, float error, float dt);
