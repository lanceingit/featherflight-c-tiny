#include "board.h"
#include <math.h>

#include "timer.h"
#include "i2c.h"
#include "ms5611.h"
#include "sensor.h"
#include "mathlib.h"


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




#define MS5611_CONVERSION_INTERVAL	25000	/* microseconds */
#define MS5611_MEASUREMENT_RATIO	3	/* pressure measurements per temperature measurement */
#define MSL_PRESSURE   101325




struct ms5611_s ms5611 = {
	.heir = {
		.init = &ms5611_init,
		.update = &ms5611_update,
	},
};

static struct ms5611_s* this=&ms5611;

static bool ms5611_read_prom(void);
static int8_t ms5611_crc(uint16_t *prom);
static uint32_t ms5611_read_adc(void);



bool ms5611_init(void)
{
    this->i2c = i2c_open(MS5611_I2C);

    int8_t ret;
    uint8_t sig;

    i2c_write(this->i2c, MS5611_ADDR, CMD_RESET, 1);
    delay_ms(100);

    
    ret = i2c_read(this->i2c, MS5611_ADDR, CMD_PROM_RD, 1, &sig);
    if (ret < 0)
        return false;

    if(ms5611_read_prom() == false)
    {
        return false;
    }        
    
    this->measure_phase = 0;
    this->collect_phase = false;
    
    return true;
}


void ms5611_update()
{
    if (this->collect_phase)
    {
        uint32_t raw;
        raw = ms5611_read_adc();

        if(this->measure_phase == 0)
        {
            int32_t dT = (int32_t)raw - ((int32_t)this->prom_buf.s.c5_reference_temp << 8);
            this->TEMP = 2000 + (int32_t)(((int64_t)dT * this->prom_buf.s.c6_temp_coeff_temp) >> 23);
			this->OFF  = ((int64_t)this->prom_buf.s.c2_pressure_offset << 16) + (((int64_t)this->prom_buf.s.c4_temp_coeff_pres_offset * dT) >> 7);
			this->SENS = ((int64_t)this->prom_buf.s.c1_pressure_sens << 15) + (((int64_t)this->prom_buf.s.c3_temp_coeff_pres_sens * dT) >> 8);
			if (this->TEMP < 2000)
            {

				int32_t T2 = POW2(dT) >> 31;

				int64_t f = POW2((int64_t)this->TEMP - 2000);
				int64_t OFF2 = 5 * f >> 1;
				int64_t SENS2 = 5 * f >> 2;

				if (this->TEMP < -1500) {

					int64_t f2 = POW2(this->TEMP + 1500);
					OFF2 += 7 * f2;
					SENS2 += 11 * f2 >> 1;
				}

				this->TEMP -= T2;
				this->OFF  -= OFF2;
				this->SENS -= SENS2;
			}
        }
        else
        {
            int32_t P = (((raw * this->SENS) >> 21) - this->OFF) >> 15;
//            _P = P * 0.01f;
//            _T = _TEMP * 0.01f;
        
            this->heir.temperature = this->TEMP / 100.0f;
            this->heir.pressure = P / 100.0f;		/* convert to millibar */
            const double T1 = 15.0 + 273.15;	/* temperature at base height in Kelvin */
            const double a  = -6.5 / 1000;	/* temperature gradient in degrees per metre */
            const double g  = 9.80665;	/* gravity constant in m/s/s */
            const double R  = 287.05;	/* ideal gas constant in J/kg/K */

            /* current pressure at MSL in kPa */
            double p1 = MSL_PRESSURE / 1000.0;

            /* measured pressure in kPa */
            double p = P / 1000.0;

            /*
             * Solve:
             *
             *     /        -(aR / g)     \
             *    | (p / p1)          . T1 | - T1
             *     \                      /
             * h = -------------------------------  + h1
             *                   a
             */
            this->heir.altitude = (((powerf((p / p1), (-(a * R) / g))) * T1) - T1) / a;

        }
        
        this->measure_phase++;
        if(this->measure_phase >= MS5611_MEASUREMENT_RATIO+1)
        {
            this->measure_phase = 0;
        }
        
        this->collect_phase = false;
        return;
    }
    
    
    if(this->measure_phase == 0)
    {
        i2c_write(this->i2c, MS5611_ADDR, ADDR_CMD_CONVERT_D2_OSR1024, 1);
    }
    else
    {
        i2c_write(this->i2c, MS5611_ADDR, ADDR_CMD_CONVERT_D1_OSR1024, 1);
    }
        
    this->collect_phase = true;

}

static uint32_t ms5611_read_adc(void)
{
    uint8_t rxbuf[3];
    i2c_read(this->i2c, MS5611_ADDR, CMD_ADC_READ, 3, rxbuf); // read ADC
    return (rxbuf[0] << 16) | (rxbuf[1] << 8) | rxbuf[2];
}

static bool ms5611_read_prom(void)
{
    uint8_t rxbuf[2] = { 0, 0 };

    for (uint8_t i = 0; i < PROM_NB; i++)
    {
        i2c_read(this->i2c, MS5611_ADDR, CMD_PROM_RD + i * 2, 2, rxbuf); // send PROM READ command
        this->prom_buf.c[i] = rxbuf[0] << 8 | rxbuf[1];
    
    }
    
    if (ms5611_crc(this->prom_buf.c) != 0)
        return false;    
    
    return true;
}

static int8_t ms5611_crc(uint16_t *prom)
{
    int32_t i, j;
    uint32_t res = 0;
    uint8_t crc = prom[7] & 0xF;
    prom[7] &= 0xFF00;

    bool blankEeprom = true;

    for (i = 0; i < 16; i++) {
        if (prom[i >> 1]) {
            blankEeprom = false;
        }
        if (i & 1)
            res ^= ((prom[i >> 1]) & 0x00FF);
        else
            res ^= (prom[i >> 1] >> 8);
        for (j = 8; j > 0; j--) {
            if (res & 0x8000)
                res ^= 0x1800;
            res <<= 1;
        }
    }
    prom[7] |= crc;
    if (!blankEeprom && crc == ((res >> 12) & 0xF))
        return 0;

    return -1;
}



