#pragma once

#include "sensor.h"
#include "timer.h"

struct spl06_cali_s
{
    int16_t c0;
    int16_t c1;
    int32_t c00;
    int32_t c10;
    int16_t c01;
    int16_t c11;
    int16_t c20;
    int16_t c21;
    int16_t c30;       
    uint8_t id;
    int32_t kP;    
    int32_t kT;
} ;

struct spl06_report_s
{
	struct spl06_cali_s cali;
	int32_t press;
	int32_t temp;
};

struct spl06_linux_s 
{
    struct baro_s heir;
    int fd;
    struct spl06_report_s report;
    times_t last_time;
}; 

extern struct spl06_linux_s spl06_linux;

bool spl06_linux_init(void);
void spl06_linux_update(void);



