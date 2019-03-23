#pragma once

#include "mathlib.h"
#include "sensor.h"
#include "func_type.h"
#include "timer.h"
#include "commander.h"

#define SPIN_RATE_LIMIT    0.175f

typedef struct 
{
    //member
    init_func init;
    run_func run;    
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
} AttEst;

#define EST_ROLL            att->roll
#define EST_PITCH           att->pitch
#define EST_YAW             att->yaw
#define EST_ROLL_RATE       att->roll_rate
#define EST_PITCH_RATE      att->pitch_rate
#define EST_YAW_RATE        att->yaw_rate


typedef struct
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

    init_func init;
    run_func run;    
	times_t last_time;
} PosEst;

typedef struct
{
    //member
    init_func init;
    run_func run;    
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
} AltEst;

#define EST_REF_ALT             alt->ref_alt
#define EST_TERRAIN_OFFSET      alt->terrain_offset
#define EST_ALT_VEL             alt->vel
#define EST_ALT                 alt->alt

extern AttEst* att;
extern AltEst* alt;
extern PosEst* pos;

void att_est_register(AttEst* est);
void alt_est_register(AltEst* est);
void pos_est_register(PosEst* est);

void est_init(void);
void est_att_run(void);
void est_alt_run(void);
void est_pos_run(void);

#include "att_est_q.h"
#include "att_est_cf.h"
#include "alt_est_3o.h"
#include "alt_est_inav.h"
#include "pos_est_inav.h"
