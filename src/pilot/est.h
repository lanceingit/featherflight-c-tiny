#pragma once

#include "mathlib.h"
#include "sensor.h"
#include "func_type.h"
#include "timer.h"
#include "commander.h"

#define SPIN_RATE_LIMIT    0.175f

struct att_est_s 
{
    //member
    init_func* init;
    run_func* run;    
	times_t last_time;

    bool inited;
    bool valid;

    bool use_compass;
	float dt_max;

    Vector acc;
    Vector gyro;
    Vector mag;

    float roll;     //DEG
    float pitch;    //DEG
    float yaw;      //DEG

    float roll_rate;    //DEG/S
    float pitch_rate;   //DEG/S
    float yaw_rate;     //DEG/S

    Quaternion	q;
    Dcm r;
    Vector gyro_bias;
	Vector corr;
    float spin_rate;
};

struct pos_est_s 
{
    //member
    float x;		//N
	float y;		//E
	float vx;
	float vy;

	float eph;
	bool valid;
    bool inited;

	double lat;
	double lon;

	double ref_lat;
	double ref_lon;

    init_func* init;
    run_func* run;    
	times_t last_time;
};

struct alt_est_s 
{
    //member
    init_func* init;
    run_func* run;    
	times_t last_time;

    bool inited;
	float alt;		//U
	float vel;

    float acc_neu_z;

	float epv;
	bool valid;

    float terrain_offset;

	float ref_alt;
    bool ref_inited;
};

extern struct att_est_s* att;
extern struct alt_est_s* alt;
extern struct pos_est_s* pos;

void att_est_register(struct att_est_s* est);
void alt_est_register(struct alt_est_s* est);
void pos_est_register(struct pos_est_s* est);

void est_init(void);
void est_att_run(void);
void est_alt_run(void);
void est_pos_run(void);

#include "att_est_q.h"
#include "att_est_cf.h"
#include "alt_est_3o.h"
#include "alt_est_inav.h"
#include "pos_est_inav.h"
