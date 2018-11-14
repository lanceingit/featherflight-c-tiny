#pragma once

#include "sensor.h"
#include "i2c.h"

struct hmc5883_s 
{
    struct compass_s heir;
    struct i2c_s* i2c;
};

extern struct hmc5883_s hmc5883;

bool hmc5883_init(void);
void hmc5883_update(void);






