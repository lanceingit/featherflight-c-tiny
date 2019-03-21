#pragma once

#include "rotation.h"
#include "lpf.h"
#include "vector.h"
#include "func_type.h"


typedef struct
{
    //member
    bool ready;    
    bool is_update;   

    Vector acc;
    Vector gyro;
    Vector gyro_offset;
    bool gyro_need_cal;
    float temp;

	lpf2p_s	acc_filter_x;
	lpf2p_s	acc_filter_y;
	lpf2p_s	acc_filter_z;
	lpf2p_s	gyro_filter_x;
	lpf2p_s	gyro_filter_y;
	lpf2p_s	gyro_filter_z;

    rotation_e rotation;

    //method
    init_func* init;
    imu_update_func* update;
} imu_s;

typedef struct
{
    //member
    Vector mag;
    
    //method
    init_func* init;
    update_func* update;
} compass_s;

typedef struct
{
    //member
    float temperature;
    float pressure;
    float altitude;
    float altitude_smooth;    

    //method
    init_func* init;
    update_func* update;     
} baro_s;

extern imu_s* imu;
extern compass_s* compass;
extern baro_s* baro;

void imu_register(imu_s* item);
void imu_update(void);

void compass_register(compass_s* item);
void compass_update(void);

void baro_register(baro_s* item);
void baro_update(void);

void sensor_init(void);

#ifdef F3_EVO
#include "mpu6050.h"
#include "hmc5883.h"
#include "ms5611.h"
#elif LINUX
#include "mpu6050_linux.h"
#include "spl06_linux.h"
#include "dps280_linux.h"
#endif
