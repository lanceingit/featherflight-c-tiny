#pragma once

#include "sensor.h"
#include "timer.h"

struct dps280_cali_s
{
    int16_t C0;	
    int16_t C1;	
    int32_t	C00;	
    int32_t C10;	
    int16_t C01;
    int16_t	C11;	
    int16_t	C20;	
    int16_t	C21;	
    int16_t	C30;	
} ;

struct dps280_report_s
{
	uint8_t pressure[3];
	uint8_t temperature[3];
	uint32_t tmp_osr_scale_coeff;
	uint32_t prs_osr_scale_coeff;
	struct dps280_cali_s calib_coeffs;
};

struct dps280_linux_s 
{
    struct baro_s heir;
    int fd;
    struct dps280_report_s report;
    times_t last_time;
}; 

extern struct dps280_linux_s dps280_linux;

bool dps280_linux_init(void);
void dps280_linux_update(void);


