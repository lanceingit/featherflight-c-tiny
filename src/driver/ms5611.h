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

typedef struct
{
    Baro heir;
    union prom_u prom_buf;
    uint8_t measure_phase;
    uint8_t collect_phase;
    int32_t TEMP;
    int32_t OFF;
    int32_t SENS;    
	I2c* i2c;
} Ms5611; 

extern Ms5611 ms5611;

bool ms5611_init(void);
void ms5611_update(void);


#define MS5611_ADDR                 0x77

#define CMD_RESET               0x1E // ADC reset command
#define CMD_ADC_READ            0x00 // ADC read command
#define CMD_ADC_CONV            0x40 // ADC conversion command
#define CMD_ADC_D1              0x00 // ADC D1 conversion
#define CMD_ADC_D2              0x10 // ADC D2 conversion
#define CMD_ADC_256             0x00 // ADC OSR=256
#define CMD_ADC_512             0x02 // ADC OSR=512
#define CMD_ADC_1024            0x04 // ADC OSR=1024
#define CMD_ADC_2048            0x06 // ADC OSR=2048
#define CMD_ADC_4096            0x08 // ADC OSR=4096
#define CMD_PROM_RD             0xA0 // Prom read command
#define PROM_NB                 8

#define ADDR_CMD_CONVERT_D1_OSR256		0x40	/* write to this address to start pressure conversion */
#define ADDR_CMD_CONVERT_D1_OSR512		0x42	/* write to this address to start pressure conversion */
#define ADDR_CMD_CONVERT_D1_OSR1024		0x44	/* write to this address to start pressure conversion */
#define ADDR_CMD_CONVERT_D1_OSR2048		0x46	/* write to this address to start pressure conversion */
#define ADDR_CMD_CONVERT_D1_OSR4096		0x48	/* write to this address to start pressure conversion */
#define ADDR_CMD_CONVERT_D2_OSR256		0x50	/* write to this address to start temperature conversion */
#define ADDR_CMD_CONVERT_D2_OSR512		0x52	/* write to this address to start temperature conversion */
#define ADDR_CMD_CONVERT_D2_OSR1024		0x54	/* write to this address to start temperature conversion */
#define ADDR_CMD_CONVERT_D2_OSR2048		0x56	/* write to this address to start temperature conversion */
#define ADDR_CMD_CONVERT_D2_OSR4096		0x58	/* write to this address to start temperature conversion */

#define T_BASE	(15.0f + 273.15f)	/* temperature at base height in Kelvin */
#define T_GRA	(-6.5f / 1000.0f)	/* temperature gradient in degrees per metre */
#define R_GAS   (287.05f)	/* ideal gas constant in J/kg/K */

#define MS5611_CONVERSION_INTERVAL	25000	/* microseconds */
#define MS5611_MEASUREMENT_RATIO	3	/* pressure measurements per temperature measurement */
#define MSL_PRESSURE   101325.0f

