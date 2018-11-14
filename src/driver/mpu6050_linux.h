#pragma once

#include "rotation.h"
#include "sensor.h"

typedef struct{
	int16_t acc[3];
	int16_t temp;
	int16_t gyro[3];
} mpu6050_report_s;

struct mpu6050_linux_s
{
	struct imu_s heir;
	Vector gyro_raw;
    mpu6050_report_s report;   
    int fd; 
};

extern struct mpu6050_linux_s mpu6050_linux;

bool mpu6050_linux_init(void);
void mpu6050_linux_update(Vector* acc, Vector* gyro);


