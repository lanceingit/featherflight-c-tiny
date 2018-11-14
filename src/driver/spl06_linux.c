#include "board.h"
#include <fcntl.h>
#include <unistd.h>

#include "spl06_linux.h"
#include "sensor.h"
#include "timer.h"
#include "mathlib.h"

#define SPL06_UPDATE_RATE	(40*1000)

struct spl06_linux_s spl06_linux = {
	.heir = {
		.init = &spl06_linux_init,
		.update = &spl06_linux_update,
	},
    .fd = -1,
	.last_time = 0,
};

static struct spl06_linux_s* this=&spl06_linux;


bool spl06_linux_init(void)
{
	this->fd = open(SPL06_PATH,O_RDONLY);
	if(this->fd < 0){
		return false;
	}

	return true;
}

void spl06_convert(struct spl06_report_s *rp,float * press,float *temp)
{
    float tcompensate;
	float pcompensate;
	float tsc; 
    float psc;
	float qua2; 
    float qua3;

	tsc = rp->temp / (float)rp->cali.kT;
	psc = rp->press / (float)rp->cali.kP;

	tcompensate = rp->cali.c0 * 0.5f + rp->cali.c1 * tsc;

	qua2 = rp->cali.c10 + psc * (rp->cali.c20 + psc* rp->cali.c30);
	qua3 = tsc * psc * (rp->cali.c11 + psc * rp->cali.c21);
	pcompensate = rp->cali.c00 + psc * qua2 + tsc * rp->cali.c01 + qua3;

	*press = pcompensate / 100.0f;
	*temp = tcompensate;
}

void spl06_linux_update()
{
	if(timer_check(&this->last_time, SPL06_UPDATE_RATE)) {
		if(read(this->fd,&this->report,sizeof(this->report)) > 0) {
			spl06_convert(&this->report, &this->heir.pressure, &this->heir.temperature);

			this->heir.altitude = press2alt(this->heir.pressure);
		}
	}
}




