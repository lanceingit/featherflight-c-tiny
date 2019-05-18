#include "board.h"
#include "failsafe.h"
#include "param.h"
#include "debug.h"
#include "timer.h"
#include <math.h>

#include "battery.h"
#include "est.h"

Failsafe failsafe = {
    .is_low_power = false,
    .is_overturn = false,
    .is_link_lose = false,
};

static Failsafe* this=&failsafe;


void failsafe_link_keep(void)
{
    this->link_time = timer_new(*this->link_timeout*1000*1000);
}

void failsafe_update(void)
{
    if(batt.volt < *this->low_power_threshold) {
        this->is_low_power = true;
    } 
    else {
        this->is_low_power = false;
    }
    
    if(fabsf(EST_ROLL) > *this->overturn_threshold || fabsf(EST_PITCH) > *this->overturn_threshold) {
        this->is_overturn = true;
    } 
    else {
        this->is_overturn = false;
    }    
    
    if(timer_is_timeout(&this->link_time)) {
        if(!this->is_link_lose) {
            PRINT("link lose!\n");
        }
        this->is_link_lose = true;
    } 
    else {
        if(this->is_link_lose) {
            PRINT("link connect!\n");
        }
        this->is_link_lose = false;
    }
}
        
void failsafe_init(void)
{
	PRINT("failsafe init!\n");
    this->low_power_threshold = PARAM_POINT(FS_BATT_LOW);
    this->overturn_threshold = PARAM_POINT(FS_RP_MAX);
    this->link_timeout = PARAM_POINT(FS_LINK_LOSE);
    
    this->link_time = timer_new(*this->link_timeout*1000*1000);
}
