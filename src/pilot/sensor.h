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

	Lpf2p	acc_filter_x;
	Lpf2p	acc_filter_y;
	Lpf2p	acc_filter_z;
	Lpf2p	gyro_filter_x;
	Lpf2p	gyro_filter_y;
	Lpf2p	gyro_filter_z;

    rotation_e rotation;

    //method
    init_func init;
    imu_update_func update;
} Imu;

#define SENS_IMU_IS_READY   imu->ready 
#define SENS_IMU_IS_UPDATE  imu->is_update
#define SENS_GYRO           imu->gyro
#define SENS_ACC            imu->acc
#define SENS_IMU_TEMP       imu->temp


typedef struct
{
    //member
    Vector mag;
    
    //method
    init_func init;
    update_func update;
} Compass;

#define SENS_MAG      compass->mag


typedef struct
{
    //member
    float temperature;
    float pressure;
    float altitude;
    float altitude_smooth;    

    //method
    init_func init;
    update_func update;     
} Baro;

#define SENS_BARO_TEMP    baro->temperature
#define SENS_PRESS        baro->pressure
#define SENS_BARO_ALT     baro->altitude
#define SENS_BARO_ALT_S   baro->altitude_smooth



extern Imu* imu;
extern Compass* compass;
extern Baro* baro;

void imu_register(Imu* item);
void imu_update(void);

void compass_register(Compass* item);
void compass_update(void);

void baro_register(Baro* item);
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
