#pragma once

enum alt_scene_e
{
    ALT_NORMAL = 0,      
    ALT_PRE_TAKEOFF,
    ALT_TAKEOFF,     
    ALT_MOVE_UP,     
    ALT_MOVE_DOWN,     
};


struct stick_s
{
    float roll;
    float pitch;
    float yaw;
    float thrust;
};

bool system_armed(void);

enum alt_scene_e commader_get_alt_scene(void);

float commander_get_roll(void);
float commander_get_pitch(void);
float commander_get_yaw(void);
float commander_get_thrust(void);
float commander_get_yaw_rate(void);
float commander_get_vel_z(void);

void commander_set_roll(uint8_t ch, float v);
void commander_set_pitch(uint8_t ch, float v);
void commander_set_yaw(uint8_t ch, float v);
void commander_set_thrust(uint8_t ch, float v);

void commander_init(void);
void commander_update(void);

