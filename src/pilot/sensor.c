#include "board.h"

#include "sensor.h"
#include "timer.h"
#include "commander.h"
#include "debug.h"
#include "lpf.h"

struct imu_s* imu;
struct compass_s* compass;
struct baro_s* baro;

void imu_gyro_cal(void)         
{
#define GYRO_CAL_STATUS_COLLECT  	0
#define GYRO_CAL_STATUS_CAL  		1
#define GYRO_CAL_STATUS_IDEL  		2

#define COLLECT_MAX  50

	static uint8_t status = GYRO_CAL_STATUS_COLLECT;
	static uint8_t collect_cnt=0;

	Vector gyro;
	static Vector gyro_sum;
	static Vector accel_start;
	Vector accel_end;
	Vector accel_diff;
	static times_t cal_time=0;

	if(status == GYRO_CAL_STATUS_COLLECT) {
		if(!system_armed()) {
		    if(timer_check(&cal_time, 10*1000)) {
				accel_end = imu->acc;
				accel_diff = vector_sub(accel_start, accel_end);
				if(vector_length(accel_diff)<0.2f) { 
					if(collect_cnt < COLLECT_MAX) {
						gyro = vector_add(imu->gyro, imu->gyro_offset);
						gyro_sum = vector_add(gyro_sum, gyro);
						collect_cnt++;
					} else {
						status = GYRO_CAL_STATUS_CAL;
					}		
				} else {
					accel_start = imu->acc;
					gyro_sum = vector_set(0,0,0);			
					collect_cnt = 0;		
				}
			}
		} else {
			status = GYRO_CAL_STATUS_IDEL;
		}
	} else if(status == GYRO_CAL_STATUS_CAL) {
		imu->gyro_offset = vector_set(gyro_sum.x/COLLECT_MAX, gyro_sum.y/COLLECT_MAX, gyro_sum.z/COLLECT_MAX);
		status = GYRO_CAL_STATUS_IDEL;
		imu->gyro_need_cal = false;
		PRINT("gyro cal done\n");
        PRINT("gyro cal: %f %f %f\n", (double)imu->gyro_offset.x, 
									  (double)imu->gyro_offset.y, 
									  (double)imu->gyro_offset.z);
	} else if(status == GYRO_CAL_STATUS_IDEL) {
		if(system_armed()) {
			imu->gyro_need_cal = true;
		}

		if(!system_armed() && timer_check(&cal_time, 60*1000*1000)) {
			imu->gyro_need_cal = true;
		}

		if(!system_armed() && imu->gyro_need_cal) {
			status = GYRO_CAL_STATUS_COLLECT;
			accel_start = imu->acc;
			gyro_sum = vector_set(0,0,0);
			collect_cnt = 0;		
		}
	}
}

void imu_register(struct imu_s* item)
{
    imu = item;
}

void imu_update(void)
{
	Vector acc; 
	Vector gyro;

	imu->update(&acc, &gyro);

    rotate_3f(imu->rotation, &acc.x, &acc.y, &acc.z);
    imu->acc.x = lpf2p_apply(&imu->acc_filter_x, acc.x);
    imu->acc.y = lpf2p_apply(&imu->acc_filter_y, acc.y);
    imu->acc.z = lpf2p_apply(&imu->acc_filter_z, acc.z);

    rotate_3f(imu->rotation, &gyro.x, &gyro.y, &gyro.z);
	imu_gyro_cal();
    gyro = vector_sub(gyro, imu->gyro_offset);

	imu->gyro.x = gyro.x;
	imu->gyro.y = gyro.y;
	// imu->gyro.x = lpf2p_apply(&imu->gyro_filter_x, gyro.x);
	// imu->gyro.y = lpf2p_apply(&imu->gyro_filter_y, gyro.y);
	imu->gyro.z = lpf2p_apply(&imu->gyro_filter_z, gyro.z);

	imu->is_update = true;
}

void compass_register(struct compass_s* item)
{
    compass = item;	
}

void compass_update(void)
{
	compass->update();
}

void baro_register(struct baro_s* item)
{
    baro = item;		
}

void baro_update(void)
{
	baro->update();
	baro->altitude_smooth = lpfrc_apply(baro->altitude_smooth, baro->altitude, 0.7f);
}

void sensor_init(void)
{
	lpf2p_init(&imu->acc_filter_x, MPU6050_ACCEL_DEFAULT_RATE, MPU6050_ACCEL_XY_DEFAULT_FILTER_FREQ);
	lpf2p_init(&imu->acc_filter_y, MPU6050_ACCEL_DEFAULT_RATE, MPU6050_ACCEL_XY_DEFAULT_FILTER_FREQ);
	lpf2p_init(&imu->acc_filter_z, MPU6050_ACCEL_DEFAULT_RATE, MPU6050_ACCEL_Z_DEFAULT_FILTER_FREQ);
	lpf2p_init(&imu->gyro_filter_x, MPU6050_GYRO_DEFAULT_RATE, MPU6050_GYRO_XY_DEFAULT_FILTER_FREQ);
	lpf2p_init(&imu->gyro_filter_y, MPU6050_GYRO_DEFAULT_RATE, MPU6050_GYRO_XY_DEFAULT_FILTER_FREQ);
	lpf2p_init(&imu->gyro_filter_z, MPU6050_GYRO_DEFAULT_RATE, MPU6050_GYRO_Z_DEFAULT_FILTER_FREQ);

	imu->rotation = INERTIAL_SENSOR_ROTATION;
	imu->is_update = false;
	imu->gyro_offset = vector_set(0, 0, 0);

	if(imu->init()) {
		imu->ready = true;   
		imu_update();
	} 

	compass->init();

#ifdef LINUX
	if(spl06_linux_init()) {
		baro_register(&spl06_linux.heir);
	} else if(dps280_linux_init()) {
		baro_register(&dps280_linux.heir);
	}
#else
	baro->init();	
#endif	
}
