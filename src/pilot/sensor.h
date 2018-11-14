#pragma once

#include "rotation.h"
#include "lpf.h"
#include "vector.h"
#include "func_type.h"


struct imu_s
{
    //member
    bool ready;    
    bool is_update;   

    Vector acc;
    Vector gyro;
    Vector gyro_offset;
    bool gyro_need_cal;
    float temp;

	struct lpf2p_s	acc_filter_x;
	struct lpf2p_s	acc_filter_y;
	struct lpf2p_s	acc_filter_z;
	struct lpf2p_s	gyro_filter_x;
	struct lpf2p_s	gyro_filter_y;
	struct lpf2p_s	gyro_filter_z;

    enum Rotation rotation;

    //method
    init_func* init;
    imu_update_func* update;
};

struct compass_s
{
    //member
    Vector mag;
    
    //method
    init_func* init;
    update_func* update;
};

struct baro_s
{
    //member
    float temperature;
    float pressure;
    float altitude;
    float altitude_smooth;    

    //method
    init_func* init;
    update_func* update;     
};

extern struct imu_s* imu;
extern struct compass_s* compass;
extern struct baro_s* baro;

void imu_register(struct imu_s* item);
void imu_update(void);

void compass_register(struct compass_s* item);
void compass_update(void);

void baro_register(struct baro_s* item);
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
