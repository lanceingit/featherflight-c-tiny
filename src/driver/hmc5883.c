#include "board.h"

#include "i2c.h"
#include "hmc5883.h"
#include "sensor.h"


Hmc5883 hmc5883 = {
	.heir = {
		.init = &hmc5883_init,
		.update = &hmc5883_update,
	},
};

static Hmc5883* this=&hmc5883;


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


