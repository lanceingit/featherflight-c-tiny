#include "board.h"

#include "log.h"
#include "timer.h"
#include "mtd.h"
#include "att_est_q.h"
#include "est.h"
#include "sensor.h"
#include <string.h>
#include "debug.h"


typedef uint8_t(*log_func)(uint8_t id, uint16_t rate, void* pkt);

typedef struct {
	uint8_t type;
	uint8_t length;		// full packet length including header
	char name[NAME_SIZE];
	char format[FORMAT_SIZE];
	char labels[LABELS_SIZE];
} LogFormat;

typedef struct {
	LogFormat format;
	uint16_t rate;
	log_func pack;
} LogList;


Log logger;
static Log* this = &logger; 


void log_shell(int argc, char *argv[]);

uint8_t log_att_write(uint8_t id, uint16_t rate, void* p)
{
    struct log_att_s pkt = {
    	LOG_PACKET_HEADER_INIT(id),
		.roll = EST_ROLL,
		.pitch = EST_PITCH,
		.yaw = EST_YAW,
		.roll_sp = 0.0f,
		.pitch_sp = 0.0f,
		.yaw_sp = 0.0f,
		.roll_rate = EST_ROLL_RATE,
		.pitch_rate = EST_PITCH_RATE,
		.yaw_rate = EST_YAW_RATE,
		.roll_rate_sp = 0.0f,
		.pitch_rate_sp = 0.0f,
		.yaw_rate_sp = 0.0f,
    };
	uint8_t len = sizeof(struct log_att_s);
	memcpy(p, &pkt, len);

	return len;
}

uint8_t log_imu_write(uint8_t id, uint16_t rate, void* p)
{
    struct log_imu_s pkt = {
    	LOG_PACKET_HEADER_INIT(id),
		.acc_x = SENS_ACC.x,
		.acc_y = SENS_ACC.y,
		.acc_z = SENS_ACC.z,
		.gyro_x = SENS_GYRO.x,
		.gyro_y = SENS_GYRO.y,
		.gyro_z = SENS_GYRO.z,
		.mag_x = SENS_MAG.x,
		.mag_y = SENS_MAG.y,
		.mag_z = SENS_MAG.z,
		.temp_acc = SENS_IMU_TEMP,
		.temp_gyro = SENS_IMU_TEMP,
		.temp_mag = 0.0f,
    };
	uint8_t len = sizeof(struct log_imu_s);
	memcpy(p, &pkt, len);

	return len;
}

uint8_t log_sens_write(uint8_t id, uint16_t rate, void* p)
{
    struct log_sens_s pkt = {
    	LOG_PACKET_HEADER_INIT(id),
		.baro_pres = SENS_PRESS,
		.baro_alt =  SENS_BARO_ALT,
		.baro_temp = SENS_BARO_TEMP,
    };
	uint8_t len = sizeof(struct log_sens_s);
	memcpy(p, &pkt, len);

	return len;
}

// uint8_t log_alt_write(uint8_t id, uint16_t rate, void* p)
// {
//     struct log_alt_s pkt = {
//     	LOG_PACKET_HEADER_INIT(id),
// 		.pos = alt_est_3o.heir.alt,
// 		.vel= alt_est_3o.heir.vel,
// 		// .baro_alt;
// 		// .baro_vel;
// 		.baro_corr = alt_est_3o.alt_err,
// 		// .acc_alt;
// 		// .acc_vel;
// 		.acc = alt_est_3o.heir.acc_neu_z,
// 		.bias = alt_est_3o.acc_corr,
// 		.alt_ref = alt_est_3o.heir.ref_alt,
// 		.baro = baro->altitude_smooth - alt_est_3o.heir.ref_alt,
//     };
// 	uint8_t len = sizeof(struct log_alt_s);
// 	memcpy(p, &pkt, len);

// 	return len;
// }

#include "log_messages.h"

static LogList log_list[] = {
	LOG_DEF(att, 50, "ffffffffffff",	"r,p,y,rsp,psp,ysp,rr,pr,yr,rrs,prs,yrs"),
	LOG_DEF(imu, 100, "ffffffffffff", "AccX,AccY,AccZ,GyroX,GyroY,GyroZ,MagX,MagY,MagZ,tA,tG,tM"),
	LOG_DEF(sens, 50, "fff", "BaroPres,BaroAlt,BaroTemp"),
	// LOG_DEF(alt, 50, "fffffffffff","alt,vel,bp,bv,bc,ap,av,a,b,ref,baro"),
};

static const uint8_t log_num = sizeof(log_list) / sizeof(log_list[0]);
static times_t timer[sizeof(log_list) / sizeof(log_list[0])];

uint32_t log_get_size(void)
{
	return mtd_get_store();
}

void log_stop(void)
{
	this->record = false;
}

bool log_need_record(void)
{
	return this->record;
}

void log_init()
{
	cli_regist("log", log_shell);
	this->record = true;

	struct {
		uint8_t head1;
        uint8_t head2;
        uint8_t msg_type;
		LogFormat body;
	} log_msg_format = {
		.head1 = HEAD_BYTE1, 
		.head2 = HEAD_BYTE2, 
		.msg_type = LOG_FORMAT_MSG,
	};

	/* fill message format packet for each format and write it */
	for (uint8_t i = 0; i < log_num; i++) {
		log_list[i].format.type = i;
		log_msg_format.body = log_list[i].format;

		log_msg_format.body.format[0] = 'I';
		memcpy(&log_msg_format.body.format[1], log_list[i].format.format, FORMAT_SIZE-1);
		log_msg_format.body.format[FORMAT_SIZE-1] = '\0';
		
		log_msg_format.body.labels[0] = 'M';
		log_msg_format.body.labels[1] = 's';
		log_msg_format.body.labels[2] = ',';
		memcpy(&log_msg_format.body.labels[3], log_list[i].format.labels, LABELS_SIZE-3);
		log_msg_format.body.labels[FORMAT_SIZE-1] = '\0';

		log_write(&log_msg_format, sizeof(log_msg_format));
	}
}

void log_write(void* pkt, uint16_t len)
{
    mtd_write((uint8_t*)pkt, len);
}

int32_t log_read(uint32_t offset, uint8_t* data, uint16_t len)
{
	return mtd_read(offset, data, len);
}

void log_run(void)
{
	// if(timer_now() < 3*1000*1000) return; 
	if(!mtd_is_full() && log_need_record()) {
		for(uint8_t i=0; i<log_num; i++) {
			if(log_list[i].rate != 0) {
				if(timer_check(&timer[log_list[i].format.type], 1*1000*1000/log_list[i].rate)) {
					uint8_t len = log_list[i].pack(log_list[i].format.type, log_list[i].rate, (void*)this->buf);    
					// PRINT("log_write len:%d\n", len);
					log_write(this->buf, len);
				}
			}
		}
	}
	mtd_sync();
}


void log_shell(int argc, char *argv[])
{
	if(argc == 2) {
		if(strcmp(argv[1],"list") == 0) {
			PRINT("NAME\t\tID\t\tRATE/hz\t\tlen\n");
			for(uint8_t i=0; i<log_num; i++) {
				PRINT("[%s]\t\t%d\t\t%d\t\t%d\n", log_list[i].format.name, log_list[i].format.type, log_list[i].rate, log_list[i].format.length);
			}
			return;
		}
	}
	cli_device_write("missing command: try 'list' ");
}







