#include "board.h"

#include "i2c.h"
#include "hmc5883.h"
#include "sensor.h"


#define MAG_ADDRESS 0x1E
#define MAG_DATA_REGISTER 0x03
#define HMC58X3_R_CONFA 0
#define HMC58X3_R_CONFB 1
#define HMC58X3_R_MODE 2
#define HMC58X3_X_SELF_TEST_GAUSS (+1.16f)       // X axis level when bias current is applied.
#define HMC58X3_Y_SELF_TEST_GAUSS (+1.16f)       // Y axis level when bias current is applied.
#define HMC58X3_Z_SELF_TEST_GAUSS (+1.08f)       // Z axis level when bias current is applied.
#define SELF_TEST_LOW_LIMIT  (243.0f / 390.0f)    // Low limit when gain is 5.
#define SELF_TEST_HIGH_LIMIT (575.0f / 390.0f)    // High limit when gain is 5.
#define HMC_POS_BIAS 1
#define HMC_NEG_BIAS 2

struct hmc5883_s hmc5883 = {
	.heir = {
		.init = &hmc5883_init,
		.update = &hmc5883_update,
	},
};

static struct hmc5883_s* this=&hmc5883;



bool hmc5883_init(void)
{
    int8_t ret;
    uint8_t sig = 0;

    this->i2c = i2c_open(HMC5883_I2C);

    ret = i2c_read(this->i2c, MAG_ADDRESS, 0x0A, 1, &sig);
    if (ret<0 || sig != 'H')
        return false;
    
    
    i2c_write(this->i2c, MAG_ADDRESS, HMC58X3_R_CONFB, (3 << 5));
    i2c_write(this->i2c, MAG_ADDRESS, HMC58X3_R_MODE, 0x00);
    
    return true;
   
}

void hmc5883_update(void)
{
    uint8_t buf[6];

    int8_t ret = i2c_read(this->i2c, MAG_ADDRESS, MAG_DATA_REGISTER, 6, buf);
    if (ret < 0) {
        return ;
    }
    // During calibration, magGain is 1.0, so the read returns normal non-calibrated values.
    // After calibration is done, magGain is set to calculated gain values.
    this->heir.mag.x = (int16_t)(buf[0] << 8 | buf[1]) * (1.0f / 660.0f);
    this->heir.mag.y = (int16_t)(buf[2] << 8 | buf[3]) * (1.0f / 660.0f);
    this->heir.mag.z = (int16_t)(buf[4] << 8 | buf[5]) * (1.0f / 660.0f);
}


