#include "board.h"

#include "est.h"
#include "timer.h"
#include "debug.h"
#include "trigger.h"

struct att_est_s* att=NULL;
struct alt_est_s* alt=NULL;
struct pos_est_s* pos=NULL;

void att_est_register(struct att_est_s* est)
{
    att = est;
}

void att_init(void)
{
	if(att == NULL) {
		PRINT("no est att register\n");
		return;
	}
    att->use_compass = false;
	att->last_time = 0;
	att->inited = false;    
    
    att->init();   	
}

void alt_est_register(struct alt_est_s* est)
{
	alt = est;
}

void alt_init(void)
{
	if(alt == NULL) {
		PRINT("no est alt register\n");
		return;
	}

    alt->inited = false;
	alt->alt = 0.0f;		
	alt->vel = 0.0f;
	alt->valid = false;
	alt->ref_alt = 0.0f;
    alt->ref_inited = false;
	alt->init();
}

void pos_est_register(struct pos_est_s* est)
{
	pos = est;
}

void pos_init(void)
{
	if(pos == NULL) {
		PRINT("no est pos register\n");
		return;
	}

    pos->inited = false;
	pos->x = 0.0f;		
	pos->y = 0.0f;		
	pos->vx = 0.0f;
	pos->vy = 0.0f;
	pos->valid = false;
	pos->init();
}

void est_init(void)
{
	att_init();
	alt_init();
	pos_init();
}

void est_att_run(void)
{
	if(imu->ready) {
		att->gyro = imu->gyro;
		att->acc = imu->acc;

		if (vector_length(att->acc) < 0.01f) {
			PRINT("WARNING: degenerate accel!\n");
			return;
		}
	} else {
		return;
	}

	if(att->use_compass) {
        att->mag = compass->mag;

		if (vector_length(att->mag) < 0.01f) {
			PRINT("WARNING: degenerate mag!\n");
			return;
		}
    }

	float dt = timer_get_dt(&att->last_time, 0.02f, 0.00001f);

	if (!att->run(dt)) {
		return;
	}

    Vector euler = quaternion_to_euler(att->q);  

    att->roll_rate =  (imu->gyro.x + att->gyro_bias.x)*M_RAD_TO_DEG;
    att->pitch_rate = (imu->gyro.y + att->gyro_bias.y)*M_RAD_TO_DEG;
    att->yaw_rate =   (imu->gyro.z + att->gyro_bias.z)*M_RAD_TO_DEG;

    att->roll = euler.x*M_RAD_TO_DEG;
    att->pitch = euler.y*M_RAD_TO_DEG;
    att->yaw = euler.z*M_RAD_TO_DEG;    

	quaternion_to_dcm(att->q, att->r);
}


void est_alt_run(void)
{
	#define BARO_CAL_MAX  500
	static uint16_t baro_read_cnt=0;

	if(!alt->ref_inited) {
		if(baro_read_cnt < BARO_CAL_MAX) {
			alt->ref_alt += baro->altitude; 
			baro_read_cnt++;
		} else {
			alt->ref_alt /= BARO_CAL_MAX;
			alt->ref_inited = true; 
		}
		return;
	}

	float dt = timer_get_dt(&alt->last_time, 0.02f, 0.001f);

	alt->run(dt);

}

void est_pos_run(void)
{
	float dt = timer_get_dt(&pos->last_time, 0.02f, 0.001f);

	pos->run(dt);
}
