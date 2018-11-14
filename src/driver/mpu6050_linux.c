#include "board.h"
#include <fcntl.h>
#include <unistd.h>

#include "mpu6050_linux.h"
#include "lpf.h"
#include "sensor.h"


#define ACC_SCALE  0.002387768f 
#define GYRO_SCALE 0.001065185f 

struct mpu6050_linux_s mpu6050_linux = {
	.heir = {
		.init = &mpu6050_linux_init,
		.update = &mpu6050_linux_update,
		.ready = false,
	},
    .fd = -1,
};

static struct mpu6050_linux_s* this=&mpu6050_linux;


bool mpu6050_linux_init(void)
{	
	this->fd = open(MPU6050_PATH, O_RDONLY);
	if(this->fd < 0){
		return false;
	}

	return true;
}

void mpu6050_linux_update(Vector* acc, Vector* gyro)
{
	if(read(this->fd, &this->report, sizeof(this->report)) <= 0) return;	

	this->heir.temp = 36.53f + (float)(this->report.temp) / 340.0f;

    acc->x = (float)(this->report.acc[0]* ACC_SCALE),
	acc->y = (float)(this->report.acc[1]* ACC_SCALE),
	acc->z = (float)(this->report.acc[2]* ACC_SCALE),
        
    gyro->x = (float)(this->report.gyro[0]* GYRO_SCALE),
    gyro->y = (float)(this->report.gyro[1]* GYRO_SCALE),
    gyro->z = (float)(this->report.gyro[2]* GYRO_SCALE),

    this->gyro_raw = *gyro;
}



