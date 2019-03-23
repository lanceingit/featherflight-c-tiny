#pragma once

#include "sensor.h"
#include "i2c.h"

typedef struct
{
    Compass heir;
    I2c* i2c;
} Hmc5883;

extern Hmc5883 hmc5883;

bool hmc5883_init(void);
void hmc5883_update(void);


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



