#include "board.h"

#include "log.h"
#include "timer.h"
#include "mtd.h"
#include "att_est_q.h"
#include "est.h"
#include "sensor.h"
#include <string.h>
#include "debug.h"

#define NAME_SIZE		5
#define FORMAT_SIZE		16
#define LABELS_SIZE		64

#define HEAD_BYTE1  0xA3    // Decimal 163
#define HEAD_BYTE2  0x95    // Decimal 149

typedef uint8_t(*log_func)(uint8_t id, uint16_t rate, void* pkt);

struct log_format_s 
{
	uint8_t type;
	uint8_t length;		// full packet length including header
	char name[NAME_SIZE];
	char format[FORMAT_SIZE];
	char labels[LABELS_SIZE];
};

struct log_s 
{
	struct log_format_s format;
	uint16_t rate;
	log_func pack;
};



static bool record;
static uint8_t buf[200];

void log_shell(int argc, char *argv[]);

uint8_t log_att_write(uint8_t id, uint16_t rate, void* p)
{
    struct log_att_s pkt = {
    	LOG_PACKET_HEADER_INIT(id),
		.roll = att->roll,
		.pitch = att->pitch,
		.yaw = att->yaw,
		.roll_sp = 0.0f,
		.pitch_sp = 0.0f,
		.yaw_sp = 0.0f,
		.roll_rate = att->roll_rate,
		.pitch_rate = att->pitch_rate,
		.yaw_rate = att->yaw_rate,
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
		.acc_x = imu->acc.x,
		.acc_y = imu->acc.y,
		.acc_z = imu->acc.z,
		.gyro_x = imu->gyro.x,
		.gyro_y = imu->gyro.y,
		.gyro_z = imu->gyro.z,
		.mag_x = compass->mag.x,
		.mag_y = compass->mag.y,
		.mag_z = compass->mag.z,
		.temp_acc = imu->temp,
		.temp_gyro = imu->temp,
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
		.baro_pres = baro->pressure,
		.baro_alt =  baro->altitude,
		.baro_temp = baro->temperature,
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

static struct log_s log[] = {
	LOG_DEF(att, 50, "ffffffffffff",	"r,p,y,rsp,psp,ysp,rr,pr,yr,rrs,prs,yrs"),
	LOG_DEF(imu, 100, "ffffffffffff", "AccX,AccY,AccZ,GyroX,GyroY,GyroZ,MagX,MagY,MagZ,tA,tG,tM"),
	LOG_DEF(sens, 50, "fff", "BaroPres,BaroAlt,BaroTemp"),
	// LOG_DEF(alt, 50, "fffffffffff","alt,vel,bp,bv,bc,ap,av,a,b,ref,baro"),
};

static const uint8_t log_num = sizeof(log) / sizeof(log[0]);
static times_t timer[sizeof(log) / sizeof(log[0])];

uint32_t log_get_size(void)
{
	return mtd_get_store();
}

void log_stop(void)
{
	record = false;
}

bool log_need_record(void)
{
	return record;
}

void log_init()
{
	cli_regist("log", log_shell);
	record = true;

	struct {
		uint8_t head1, head2, msg_type;
		struct log_format_s body;
	} log_msg_format = {
		.head1 = HEAD_BYTE1, 
		.head2 = HEAD_BYTE2, 
		.msg_type = LOG_FORMAT_MSG,
	};

	/* fill message format packet for each format and write it */
	for (uint8_t i = 0; i < log_num; i++) {
		log[i].format.type = i;
		log_msg_format.body = log[i].format;

		log_msg_format.body.format[0] = 'I';
		memcpy(&log_msg_format.body.format[1], log[i].format.format, FORMAT_SIZE-1);
		log_msg_format.body.format[FORMAT_SIZE-1] = '\0';
		
		log_msg_format.body.labels[0] = 'M';
		log_msg_format.body.labels[1] = 's';
		log_msg_format.body.labels[2] = ',';
		memcpy(&log_msg_format.body.labels[3], log[i].format.labels, LABELS_SIZE-3);
		log_msg_format.body.labels[FORMAT_SIZE-1] = '\0';

		log_write(&log_msg_format, sizeof(log_msg_format));
	}
}

void log_write(void* pkt, uint16_t len)
{
    mtd_write((uint8_t*)pkt, len);
}

uint16_t log_read(uint32_t offset, uint8_t* data, uint16_t len)
{
	return mtd_read(offset, data, len);
}

void log_run(void)
{
	// if(timer_now() < 3*1000*1000) return; 
	if(!mtd_is_full() && log_need_record()) {
		for(uint8_t i=0; i<log_num; i++) {
			if(log[i].rate != 0) {
				if(timer_check(&timer[log[i].format.type], 1*1000*1000/log[i].rate)) {
					uint8_t len = log[i].pack(log[i].format.type, log[i].rate, (void*)buf);    
					// PRINT("log_write len:%d\n", len);
					log_write(buf, len);
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
				PRINT("[%s]\t\t%d\t\t%d\t\t%d\n", log[i].format.name, log[i].format.type, log[i].rate, log[i].format.length);
			}
			return;
		}
	}
	cli_device_write("missing command: try 'list' ");
}







