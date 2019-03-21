#pragma once

#include "sensor.h"
#include "i2c.h"

typedef struct
{
    compass_s heir;
    i2c_s* i2c;
} hmc5883_s ;

extern hmc5883_s hmc5883;

bool hmc5883_init(void);
void hmc5883_update(void);






