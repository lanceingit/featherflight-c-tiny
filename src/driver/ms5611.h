#pragma once

#include "sensor.h"
#include "i2c.h"

struct prom_s {
	uint16_t factory_setup;
	uint16_t c1_pressure_sens;
	uint16_t c2_pressure_offset;
	uint16_t c3_temp_coeff_pres_sens;
	uint16_t c4_temp_coeff_pres_offset;
	uint16_t c5_reference_temp;
	uint16_t c6_temp_coeff_temp;
	uint16_t serial_and_crc;
};

union prom_u {
	uint16_t c[8];
	struct prom_s s;
};

struct ms5611_s 
{
    struct baro_s heir;
    union prom_u prom_buf;
    uint8_t measure_phase;
    uint8_t collect_phase;
    int32_t TEMP;
    int32_t OFF;
    int32_t SENS;    
	struct i2c_s* i2c;
}; 

extern struct ms5611_s ms5611;

bool ms5611_init(void);
void ms5611_update(void);



