#include "board.h"

#include "sensor.h"
#ifdef F3_EVO
#include "spi_flash.h"
#elif LINUX
#include <unistd.h>
#include <pthread.h>
#include <termios.h>
#include <sys/mman.h>
#include <errno.h>
#include <sched.h>
#endif
#include "mtd.h"
#include "log.h"
#include "link_mavlink.h"
#include "link_wwlink.h"
#include "mavlink_log.h"
#include "timer.h"
#include "perf.h"
#include "est.h"
#include "scheduler.h"
#include "mathlib.h"
#include "commander.h"
#include "cli.h"
#include "debug.h"
#include "navigator.h"
#include "mm.h"
#include "fifo.h"


struct variance_s baro_variance;
float baro_vari;
float baro_vel;


void fifo_test(void)
{
    struct fifo_s f;
    uint8_t buf[10];
    uint8_t i,j;
    uint8_t tmp=0;
    

    fifo_create(&f, buf, 10);

    for(i=0; i<3; i++) {
        for(j=0; j<7; j++) {
            fifo_write(&f, j);
        }
        for(j=0; j<9; j++) {
            fifo_read(&f, &tmp);
        }
    }
}


#ifdef LINUX

int linux_create_thread(const char *name, int priority, int stack_size, void* entry,void* parameter,int sched_method)
{
    int rv;
    pthread_t task;
	pthread_attr_t attr;
	struct sched_param param;
	
	rv = pthread_attr_init(&attr);
    if (rv != 0) {
		PRINT("platform_create_thread: failed to init thread attrs\n");
		return rv;
	}
	
	rv = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	if (rv != 0) {
		PRINT("platform_create_thread: failed to set inherit sched\n");
		return rv;
	}

	rv = pthread_attr_setschedpolicy(&attr, sched_method);
	if (rv != 0) {
		PRINT("platform_create_thread: failed to set sched policy\n");
		return rv;
	}
	
	if(sched_method != SCHED_OTHER){
		param.sched_priority = priority;
		rv = pthread_attr_setschedparam(&attr, &param);
		if (rv != 0) {
			PRINT("platform_create_thread: failed to set sched param\n");
			return rv;
		}
	}
	
	rv = pthread_create(&task, &attr, entry,parameter);
	if (rv != 0) {
		if (rv == EPERM) {
			PRINT("warning: unable to start");
			rv = pthread_create(&task, NULL, entry, parameter);

			if (rv != 0) {
				PRINT("platform_create_thread: failed to create thread\n");
				return (rv < 0) ? rv : -rv;
			}
		}else{
			return rv;
		}
	}
	return 0;
}
#endif

void task_link(void)
{
	mavlink_message_t msg;
    wwlink_message_t wwmsg;

	if(mavlink_recv(&msg)) {
        mavlink_log_handle(&msg);
	}

    wwlink_recv(&wwmsg);    
    mavlink_stream();
    wwlink_stream();

    PERF_DEF(link_perf)
    perf_interval(&link_perf);
    // perf_print(&link_perf, "link");
}

void task_cli(void)
{
    cli_updata();
}

void task_imu(void)
{        
    imu_update();
}    

void task_compass(void)
{
    compass_update();
}

void task_baro(void)
{
    static float baro_alt_f_old=0;
    static float baro_alt_f=0;

    baro_update();

    float alt = baro->altitude;

    baro_alt_f = ((baro_alt_f * 0.7f) + (alt * (1.0f - 0.7f)));
    baro_vel = (baro_alt_f - baro_alt_f_old) / (0.025000f);
    baro_alt_f_old = baro_alt_f;

    baro_vari = variance_cal(&baro_variance, baro_vel);
}


void task_att(void)
{
	if(imu->update) {
        PERF_DEF(att_elapsed)
		perf_begin(&att_elapsed);
		est_att_run();
		perf_end(&att_elapsed);
        // perf_print(&att_elapsed, "att_elapsed");
		imu->update = false;;
        PERF_DEF(att_perf);
		perf_interval(&att_perf);
        // perf_print(&att_perf, "att_perf");
	}
}

// void task_alt(void)
// {
//     est_alt_run();
// }

void task_commander(void)
{
    commander_update();
}

void task_navigator(void)
{
    navigator_update();
}

void task_log(void)
{
    log_run();    
}

void gyro_cal(void)         //TODO:put into sensor
{
    Vector gyro;
    Vector gyro_sum;
    Vector accel_start;
    Vector accel_end;
    Vector accel_diff;
    
	while(1) {
		imu_update();
		accel_start = imu->acc;
        gyro_sum = vector_zero();
        for(uint8_t i=0; i<50; i++) {
        	imu_update();
    		gyro = imu->gyro;
            gyro_sum = vector_add(gyro_sum, gyro);
    		delay_ms(10);
        }
        imu_update();
		accel_end = imu->acc;
        accel_diff = vector_sub(accel_start, accel_end);
		if(vector_length(accel_diff) >  0.2f) continue;

		imu->gyro_offset.x = gyro_sum.x/50;   
		imu->gyro_offset.y = gyro_sum.y/50;
		imu->gyro_offset.z = gyro_sum.z/50;
        
        return;
	}
}

#ifdef LINUX
int featherflight_thread(void* arvg);

int main()
{
    linux_create_thread("featherflight",99,10240,featherflight_thread,NULL,SCHED_FIFO);
    while(1){
	    sleep(60);
    }    
}

int featherflight_thread(void* arvg)
#else
int main() 
#endif
{    
//    PRINT("hello feather flight\n");
#ifdef F3_EVO    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    RCC_ClearFlag();
#endif        
    mm_init();
    timer_init();    
#ifdef F3_EVO    
    spi_flash_init();
#endif        
    mtd_init();
    mtd_test();
    log_init();
    mavlink_init();
    wwlink_init();
    cli_init();

#ifdef F3_EVO    
    imu_register(&mpu6050.heir);
    baro_register(&ms5611.heir);
    compass_register(&hmc5883.heir);
#elif LINUX
    imu_register(&mpu6050_linux.heir);
    // baro_register(&spl06_linux.heir);
#endif    
    task_init();
    sensor_init();

    // gyro_cal();

    fifo_test();

    att_est_register(&att_est_q.heir);
    // att_est_register(&att_est_cf.heir);
//    alt_est_register(&alt_est_3o.heir);
    // alt_est_register(&alt_est_inav.heir);
    est_init();

    variance_create(&baro_variance, 100);

    task_create("imu", 2000, task_imu);
//    task_create("compass", (10000000 / 150), task_compass);
    task_create("baro", 25000, task_baro);
    task_create("att", 2000, task_att);
    // task_create("alt", 2*1000, task_alt);
//    task_create("cmder", 2000, task_commander);
//    task_create("nav", 2000, task_navigator);
   task_create("link", 2*1000, task_link);
//    task_create("cli", 100*1000, task_cli);
   task_create("log", 10*1000, task_log);

    while(1) {

        TIMER_DEF(main_loop_time)
        float dt = timer_get_dt(&main_loop_time, 10, 0.000001);
        if(dt < 1.0f) {
            scheduler_run();
        }
        times_t main_elapsed = timer_elapsed(&main_loop_time);

    #ifdef LINUX  
        // PRINT("main loop:%3.3fms, elapsed:%3.3fms\n", dt*1000, main_elapsed/1000.0f);
		if(main_elapsed>SYSTEM_CYCLE || dt>(0.001+SYSTEM_CYCLE/1e6)) {
			PRINT("warning slow loop:%3.3fms, elapsed:%3.3fms\n", dt*1000, main_elapsed/1000.0f);
			main_elapsed = SYSTEM_CYCLE - 500;
		}
		usleep(SYSTEM_CYCLE - main_elapsed);
    #endif            

        // PERF_DEF(main_perf)
        // perf_interval(&main_perf);
        // perf_print(&main_perf, "main loop");
    }
    
    return 0;
}
