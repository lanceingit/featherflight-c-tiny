#include "board.h"
#include "commander.h"
#include "timer.h"
#include "debug.h"
#include <math.h>
#include "cmder_param.h"
#include "trigger.h"
#include "pilot.h"


#define LEFT_STICK_LEFT			(1<<0)
#define LEFT_STICK_H_CENTER	    (1<<1)
#define LEFT_STICK_RIGHT		(1<<2)
#define LEFT_STICK_UP		    (1<<3)
#define LEFT_STICK_V_CENTER	    (1<<4)
#define LEFT_STICK_DOWN			(1<<5)
#define RIGHT_STICK_LEFT		(1<<6)
#define RIGHT_STICK_H_CENTER	(1<<7)
#define RIGHT_STICK_RIGHT		(1<<8)
#define RIGHT_STICK_UP			(1<<9)
#define RIGHT_STICK_V_CENTER	(1<<10)
#define RIGHT_STICK_DOWN		(1<<11)

#define STICK_LIMIT 0.8f
#define STICK_DEADZONE 0.15f

struct commander_s
{
    bool armed;
    bool flying;
    enum alt_scene_e alt_scene;
    uint8_t curr_controler;
    Stick stick;
    float rp_gain;
    float yaw_rate_gain;
    float vel_z_gain;
};


struct commander_s commander = {
    .armed = false,
    .flying = false,
    .curr_controler = 0,
};

static struct commander_s* this = &commander;


bool system_armed(void)
{
    return this->armed;
}

enum alt_scene_e commader_get_alt_scene(void)
{
    return this->alt_scene;
}

float commander_get_roll(void)
{
    return this->stick.roll*this->rp_gain;
}

float commander_get_pitch(void)
{
    return this->stick.pitch*this->rp_gain;

}

float commander_get_yaw(void)
{
    return this->stick.yaw;

}

float commander_get_thrust(void)
{
    return this->stick.thrust;
}

float commander_get_vel_z(void)
{
    return this->stick.thrust*this->vel_z_gain;
}

float commander_get_yaw_rate(void)
{
    return this->stick.yaw*this->yaw_rate_gain;
}

void commander_set_roll(uint8_t ch, float v)
{
    if(ch > this->curr_controler) {
        this->stick.roll = v;
    }
}

void commander_set_pitch(uint8_t ch, float v)
{
    if(ch > this->curr_controler) {
        this->stick.pitch = v;
    }    
}

void commander_set_yaw(uint8_t ch, float v)
{
    if(ch > this->curr_controler) {
        this->stick.yaw = v;
    }
}

void commander_set_thrust(uint8_t ch, float v)
{
    if(ch > this->curr_controler) {
        this->stick.thrust = v;
    }
}

uint16_t stick_get_position(Stick* s, float limit, float deadzone)
{
	uint16_t pos=0;

	if(s->yaw<-limit){
		pos |= LEFT_STICK_LEFT;
	} else if (s->yaw>limit) {
		pos |= LEFT_STICK_RIGHT;
	} else if (fabsf(s->yaw)<deadzone) {
		pos |= LEFT_STICK_H_CENTER;
    }

	if(s->thrust>limit){
		pos |= LEFT_STICK_UP;
	} else if (s->thrust<-limit) {
		pos |= LEFT_STICK_DOWN;
	} else if (fabsf(s->thrust)<deadzone) {
		pos |= LEFT_STICK_V_CENTER;
    }

	if(s->roll<-limit){
		pos |= RIGHT_STICK_LEFT;
	} else if (s->roll>limit) {
		pos |= RIGHT_STICK_RIGHT;
	} else if (fabsf(s->roll)<deadzone) {
		pos |= RIGHT_STICK_H_CENTER;
    }

	if(s->pitch<-limit){
		pos |= RIGHT_STICK_UP;
	} else if (s->pitch>limit) {
		pos |= RIGHT_STICK_DOWN;
	} else if (fabsf(s->pitch)<deadzone) {
		pos |= RIGHT_STICK_V_CENTER;
    }
	return pos;
}

bool check_stick_arm(void)
{
#define WAIT_ARM_PRESS      0
#define WAIT_ARM_RELEASE    1
#define WAIT_DISARM_PRESS   2
#define WAIT_DISARM_RELEASE 3

    static times_t arm_time;
    static times_t disarm_time;
    static uint8_t status = WAIT_ARM_PRESS;
    bool armed=this->armed;

// PRINT("stick:t:%f y:%f r:%f p:%f\n", this->stick.thrust, this->stick.yaw, this->stick.roll, this->stick.p);

    if(status == WAIT_ARM_PRESS) {
        armed = false;
        if(stick_get_position(&this->stick, STICK_LIMIT, STICK_DEADZONE) == (LEFT_STICK_LEFT|LEFT_STICK_DOWN|
                                                                            RIGHT_STICK_RIGHT|RIGHT_STICK_DOWN)
        || stick_get_position(&this->stick, STICK_LIMIT, STICK_DEADZONE) == (LEFT_STICK_RIGHT|LEFT_STICK_DOWN|
                                                                            RIGHT_STICK_LEFT|RIGHT_STICK_DOWN)                                               
        ) {
            if(timer_check(&arm_time, 1500*1000)) {
                armed = true;
                status = WAIT_ARM_RELEASE;
            } 
        } else {
            arm_time = timer_now();            
        }    
    } else if(status == WAIT_ARM_RELEASE) {
        if(stick_get_position(&this->stick, STICK_LIMIT, STICK_DEADZONE) == (LEFT_STICK_V_CENTER|LEFT_STICK_H_CENTER|
                                                                            RIGHT_STICK_H_CENTER|RIGHT_STICK_V_CENTER)                                              
        ) {  
            status = WAIT_DISARM_PRESS;
        }
        armed = true;   
    } else if(status == WAIT_DISARM_PRESS) {
        armed = true;
        if(stick_get_position(&this->stick, STICK_LIMIT, STICK_DEADZONE) == (LEFT_STICK_LEFT|LEFT_STICK_DOWN|
                                                                            RIGHT_STICK_RIGHT|RIGHT_STICK_DOWN)
        || stick_get_position(&this->stick, STICK_LIMIT, STICK_DEADZONE) == (LEFT_STICK_RIGHT|LEFT_STICK_DOWN|
                                                                            RIGHT_STICK_LEFT|RIGHT_STICK_DOWN)                                               
        || stick_get_position(&this->stick, STICK_LIMIT, STICK_DEADZONE) == (LEFT_STICK_DOWN|LEFT_STICK_H_CENTER|
                                                                            RIGHT_STICK_H_CENTER|RIGHT_STICK_V_CENTER)                                               
        ) {
            if(timer_check(&disarm_time, 500*1000)) {
                armed = false;
                status = WAIT_DISARM_RELEASE;
            }
        } else {
            disarm_time = timer_now();
        }         
    } else if(status == WAIT_DISARM_RELEASE) {
        if(stick_get_position(&this->stick, STICK_LIMIT, STICK_DEADZONE) == (LEFT_STICK_V_CENTER|LEFT_STICK_H_CENTER|
                                                                            RIGHT_STICK_H_CENTER|RIGHT_STICK_V_CENTER)                                              
        ) {  
            status = WAIT_ARM_PRESS;
        }        
        armed = false;
    }
    return armed;
}

void commander_init(void)
{
    PARAM_REGISTER(cmder);
    this->rp_gain = PARAM_GET(CMDER_RP_GAIN);
    this->yaw_rate_gain = PARAM_GET(CMDER_YAW_RATE_GAIN);
    this->vel_z_gain = PARAM_GET(CMDER_VEL_Z_GAIN);
}

void commander_update(void)
{    
    bool arm_status_change=false;
    bool armed;
    if(!this->flying) {
        armed = check_stick_arm();
        if(armed != this->armed) {
            PRINT("armd change %d->%d\n", this->armed, armed);
            this->armed = armed;
            arm_status_change = true;
        }    
    }

    TIMER_DEF(alt_smooth_time)
    switch(this->alt_scene) {
        case ALT_NORMAL:
            if(arm_status_change) {
                if(this->armed) {
                    //在螺旋桨转起来前，设置气压计权重�?，关闭气压计融合�?
                    this->alt_scene = ALT_PRE_TAKEOFF;
                }
            }
            if(this->stick.thrust > STICK_DEADZONE) {
                //向上打杆。增加vel权重，加快权重转换。加速度对突然运动估计不�?
                this->alt_scene = ALT_MOVE_UP;
                alt_smooth_time = timer_new(1.2e6);
            } else if(this->stick.thrust < -STICK_DEADZONE) {
                //向下打杆。增加vel权重，比上升更大，加快权重转换�?
                this->alt_scene = ALT_MOVE_DOWN;
                alt_smooth_time = timer_new(1.2e6);
            }
            break;
        case ALT_PRE_TAKEOFF:
            //当气压计高度大于起飞前高度，设置为起飞模�?
            //pos，vel权重增加，得到一个准确值。同时减小bias权重，因为这时修正不准�?
            if(SENS_BARO_ALT_S-EST_REF_ALT > EST_TERRAIN_OFFSET) {
                this->alt_scene = ALT_TAKEOFF;
            }              
            break;
        case ALT_TAKEOFF:
//            if(navigator_get_mode() != NAV_TAKEOFF && poshold_get_mode() != POSHOLD_TAKEOFF) {
            if(navigator_get_mode() != NAV_TAKEOFF) {
                this->alt_scene = ALT_NORMAL;
            }
            break;
        case ALT_MOVE_UP:
        case ALT_MOVE_DOWN:
            if(-STICK_DEADZONE < this->stick.thrust && this->stick.thrust < STICK_DEADZONE) {
                this->alt_scene = ALT_NORMAL;
            }
            if(fabsf(EST_ALT_VEL) > PARAM_GET(CMDER_VEL_HOLD_MAX) && 
                (this->alt_scene == ALT_MOVE_UP||timer_is_timeout(&alt_smooth_time))){
                //悬停不稳时，使用速度控制。能有效抑制上拉后掉�?
                this->alt_scene = ALT_MOVE_UP;
            } else if(!timer_is_timeout(&alt_smooth_time) > 0 && this->alt_scene == ALT_MOVE_DOWN) {
                //悬停不稳时，使用位置控制，但对目标高度做平滑。能有效抑制下拉后回�?
                this->alt_scene = ALT_MOVE_DOWN;
            }        
            break;
        default:break;
    }
}
