#include "board.h"
#include <fcntl.h>
#include <unistd.h>

#include "dps280_linux.h"
#include "sensor.h"
#include "timer.h"
#include "mathlib.h"

#define DPS280_UPDATE_RATE	(40*1000)

#define POW_2_23_MINUS_1	0x7FFFFF   //implies 2^23-1
#define POW_2_24			0x1000000
#define POW_2_15_MINUS_1	0x7FFF
#define POW_2_16			0x10000
#define POW_2_11_MINUS_1	0x7FF
#define POW_2_12			0x1000
#define POW_2_20			0x100000
#define POW_2_19_MINUS_1	524287

struct dps280_linux_s dps280_linux = {
	.heir = {
		.init = &dps280_linux_init,
		.update = &dps280_linux_update,
	},
    .fd = -1,
	.last_time = 0,
};

static struct dps280_linux_s* this=&dps280_linux;


bool dps280_linux_init(void)
{
	this->fd = open(DPS280_PATH, O_RDONLY);
	if(this->fd < 0){
		return false;
	}

	return true;
}

void dps280_convert(struct dps280_report_s* rp, float* press, float* temp)
{
   double	press_raw;
   double	temp_raw;

   double 	temp_scaled;
   double 	temp_final;
   double 	press_scaled;
   double 	press_final;
   

	press_raw = (rp->pressure[2]) + (rp->pressure[1]<<8) + (rp->pressure[0] <<16);
    temp_raw  = (rp->temperature[2]) + (rp->temperature[1]<<8) + (rp->temperature[0] <<16);

	if(temp_raw > POW_2_23_MINUS_1){
		temp_raw = temp_raw - POW_2_24;
	}

	if(press_raw > POW_2_23_MINUS_1){
		press_raw = press_raw - POW_2_24;
	}

	temp_scaled = (double)temp_raw / (double) (rp->tmp_osr_scale_coeff);

	temp_final =  (rp->calib_coeffs.C0 /2.0f) + rp->calib_coeffs.C1 * temp_scaled ;
    
	press_scaled = (double) press_raw / rp->prs_osr_scale_coeff;

	press_final = rp->calib_coeffs.C00 +
                      press_scaled *  (  rp->calib_coeffs.C10 + press_scaled *
                      ( rp->calib_coeffs.C20 + press_scaled * rp->calib_coeffs.C30 )  ) +
                      temp_scaled * rp->calib_coeffs.C01 +
                      temp_scaled * press_scaled * ( rp->calib_coeffs.C11 +
                                                      press_scaled * rp->calib_coeffs.C21 );


	press_final = press_final * 0.01f;	//to convert it into mBar

	*temp = temp_final;
    *press    = press_final;  //press_final;
}

void dps280_linux_update()
{
	if(timer_check(&this->last_time, DPS280_UPDATE_RATE)) {
		if(read(this->fd,&this->report,sizeof(this->report)) > 0) {
			dps280_convert(&this->report, &this->heir.pressure, &this->heir.temperature);

			this->heir.altitude = press2alt(this->heir.pressure);
		}
	}
}




