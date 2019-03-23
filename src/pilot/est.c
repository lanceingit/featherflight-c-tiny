#include "board.h"

#include "est.h"
#include "timer.h"
#include "debug.h"
#include "trigger.h"

AttEst* att=NULL;
AltEst* alt=NULL;
PosEst* pos=NULL;

void att_est_register(AttEst* est)
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
    
    (*att->init)();   	
}

void alt_est_register(AltEst* est)
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
	(*alt->init)();
}

void pos_est_register(PosEst* est)
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
	(*pos->init)();
}

void est_init(void)
{
	att_init();
	alt_init();
	pos_init();
}

void est_att_run(void)
{
	if(SENS_IMU_IS_READY) {
		att->gyro = SENS_GYRO;
		att->acc = SENS_ACC;
        SENS_IMU_IS_UPDATE = false;

		if (vector_length(att->acc) < 0.01f) {
			PRINT("WARNING: degenerate accel!\n");
			return;
		}
	} else {
		return;
	}

	if(att->use_compass) {
        att->mag = SENS_MAG;

		if (vector_length(att->mag) < 0.01f) {
			PRINT("WARNING: degenerate mag!\n");
			return;
		}
    }

	float dt = timer_get_dt(&att->last_time, 0.02f, 0.00001f);

	if (!(*att->run)(dt)) {
		return;
	}

    Vector euler = quaternion_to_euler(att->q);  

    att->roll_rate =  (SENS_GYRO.x + att->gyro_bias.x)*M_RAD_TO_DEG;
    att->pitch_rate = (SENS_GYRO.y + att->gyro_bias.y)*M_RAD_TO_DEG;
    att->yaw_rate =   (SENS_GYRO.z + att->gyro_bias.z)*M_RAD_TO_DEG;

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
			alt->ref_alt += SENS_BARO_ALT; 
			baro_read_cnt++;
		} else {
			alt->ref_alt /= BARO_CAL_MAX;
			alt->ref_inited = true; 
		}
		return;
	}

	float dt = timer_get_dt(&alt->last_time, 0.02f, 0.001f);

	(*alt->run)(dt);

}

void est_pos_run(void)
{
	float dt = timer_get_dt(&pos->last_time, 0.02f, 0.001f);

	(*pos->run)(dt);
}
