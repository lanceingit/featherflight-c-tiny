#pragma once

void att_control_init(void);
void att_control_roll_pitch_rate_update(float dt, float roll_rate_target, float pitch_rate_target, float limit);
void att_control_roll_pitch_update(float dt, float roll_target, float pitch_target, float limit);
void att_control_yaw_rate_update(float dt, float yaw_rate_target);
void att_control_yaw_update(float dt, float yaw_target);

