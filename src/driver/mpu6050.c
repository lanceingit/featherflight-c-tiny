#include "board.h"

#include "i2c.h"
#include "timer.h"
#include "mpu6050.h"
#include "lpf.h"
#include "sensor.h"


Mpu6050 mpu6050 = {
	.heir = {
		.init = &mpu6050_init,
		.update = &mpu6050_update,
        .ready = false,
	},
};

static Mpu6050* this=&mpu6050;


bool mpu6050_init(void)
{	
    int8_t ret;
    uint8_t sig;
    
    this->i2c = i2c_open(MPU6050_I2C);

    ret = i2c_read(this->i2c, MPU6050_ADDRESS, MPU_RA_WHO_AM_I, 1, &sig);
    if(ret < 0) 
        return false;
        
    if (sig != (MPU6050_ADDRESS & 0x7e))
        return false;

    ret = i2c_write(this->i2c, MPU6050_ADDRESS, MPU_RA_PWR_MGMT_1, 0x80);      //PWR_MGMT_1    -- DEVICE_RESET 1

    delay_ms(100);
    ret = i2c_write(this->i2c, MPU6050_ADDRESS, MPU_RA_PWR_MGMT_1, 0x01); //PWR_MGMT_1    -- SLEEP 0; CYCLE 0; TEMP_DIS 0; CLKSEL 3 (PLL with Z Gyro reference)
    delay_ms(100);
    ret = i2c_write(this->i2c, MPU6050_ADDRESS, MPU_RA_PWR_MGMT_2, 0x0); //PWR_MGMT_1    -- SLEEP 0; CYCLE 0; TEMP_DIS 0; CLKSEL 3 (PLL with Z Gyro reference)
    delay_ms(100);
    ret = i2c_write(this->i2c, MPU6050_ADDRESS, MPU_RA_SMPLRT_DIV, 0); //SMPLRT_DIV    -- SMPLRT_DIV = 0  Sample Rate = Gyroscope Output Rate / (1 + SMPLRT_DIV)
     //PLL Settling time when changing CLKSEL is max 10ms.  Use 15ms to be sure
    delay_ms(15);
    ret = i2c_write(this->i2c, MPU6050_ADDRESS, MPU_RA_CONFIG, 3); //CONFIG        -- EXT_SYNC_SET 0 (disable input pin for data sync) ; default DLPF_CFG = 0 => ACC bandwidth = 260Hz  GYRO bandwidth = 256Hz)
    delay_ms(100);
    ret = i2c_write(this->i2c, MPU6050_ADDRESS, MPU_RA_GYRO_CONFIG, INV_FSR_2000DPS << 3);   //GYRO_CONFIG   -- FS_SEL = 3: Full scale set to 2000 deg/sec

    // ACC Init stuff.
    // Accel scale 8g (4096 LSB/g)
    ret = i2c_write(this->i2c, MPU6050_ADDRESS, MPU_RA_ACCEL_CONFIG, INV_FSR_16G << 3);

//    ret = i2c_write(this->i2c, MPU6050_ADDRESS, MPU_RA_ACCEL_CONFIG2, 3);
    
//    ret = i2c_write(this->i2c, MPU6050_ADDRESS, MPU_RA_INT_PIN_CFG,
//            0 << 7 | 0 << 6 | 0 << 5 | 0 << 4 | 0 << 3 | 0 << 2 | 1 << 1 | 0 << 0); // INT_PIN_CFG   -- INT_LEVEL_HIGH, INT_OPEN_DIS, LATCH_INT_DIS, INT_RD_CLEAR_DIS, FSYNC_INT_LEVEL_HIGH, FSYNC_INT_DIS, I2C_BYPASS_EN, CLOCK_DIS
//    
//    
    
    return true;
}

void mpu6050_update(Vector* acc, Vector* gyro)  
{
    if (i2c_read(this->i2c, MPU6050_ADDRESS, MPU_RA_ACCEL_XOUT_H, 14, this->buf) < 0) {
        return;
    }

    acc->x = (float)((int16_t)((this->buf[0] << 8) | this->buf[1]))*(9.80665f /2048);
    acc->y = (float)((int16_t)((this->buf[2] << 8) | this->buf[3]))*(9.80665f /2048);
    acc->z = (float)((int16_t)((this->buf[4] << 8) | this->buf[5]))*(9.80665f /2048);
    
    gyro->x = (float)((int16_t)((this->buf[ 8] << 8) | this->buf[ 9]))*(0.0174532f / 16.4f);
    gyro->y = (float)((int16_t)((this->buf[10] << 8) | this->buf[11]))*(0.0174532f / 16.4f);
    gyro->z = (float)((int16_t)((this->buf[12] << 8) | this->buf[13]))*(0.0174532f / 16.4f);  
    
    this->gyro_raw = *gyro;    
}

