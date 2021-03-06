#include "pid.h"
#include "mathlib.h"

void pid_init(Pid* self, float* p, float* i, float* d, float* i_limit, float* out_limit,
              float* d_weight)
{
    self->kp = p;
    self->ki = i;
    self->kd = d;
    self->i_limit = i_limit;
    self->out_limit = out_limit;
    self->d_weight = d_weight;
    self->error_prev = 0.0f;
    self->output = 0.0f;
    self->i_out = 0.0f;
    self->d_out = 0.0f;
}

void pid_reset(Pid* self)
{
    self->error_prev = 0.0f;
    self->output = 0.0f;
    self->i_out = 0.0f;
    self->d_out = 0.0f;
}

float pid_update(Pid* self, float error, float dt)
{
    float output = 0.0f;

    self->error = error;

    if(self->kp) {
        self->p_out = *self->kp * self->error;
    }
    if(self->ki) {
        self->i_out += *self->ki * self->error * dt;
    }
    if(self->i_limit) {
        self->i_out = constrain(self->i_out, -*self->i_limit, *self->i_limit);
    }

    if(self->kd) {
        float d = (self->error - self->error_prev) / dt;
        if(self->d_weight) {
            self->d_out = (d * (*self->d_weight) + (1.0f - *self->d_weight) * self->d_out)*(*self->kd);
        }
        else {
            self->d_out = d * (*self->kd);
        }
    }
    output = self->p_out + self->i_out + self->d_out;
    if(self->out_limit) {
        output = constrain(output, -*self->out_limit, *self->out_limit);
    }
    self->error_prev = self->error;

    return output;
}
