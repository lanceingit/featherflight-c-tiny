#pragma once

#define LOG_PACKET_HEADER	       uint8_t head1, head2, msg_type; uint32_t ms;
#define LOG_PACKET_HEADER_INIT(id) .head1 = HEAD_BYTE1, .head2 = HEAD_BYTE2, .msg_type = id, .ms = (timer_now()/1000)


typedef struct {
    bool record;
    uint8_t buf[200];
} Log;


#pragma pack(push, 1)

struct log_att_s {
	LOG_PACKET_HEADER
	float roll;
	float pitch;
	float yaw;
	float roll_sp;
	float pitch_sp;
	float yaw_sp;    
	float roll_rate;
	float pitch_rate;
	float yaw_rate;
	float roll_rate_sp;
	float pitch_rate_sp;
	float yaw_rate_sp;    
};


struct log_imu_s {
	LOG_PACKET_HEADER
	float acc_x;
	float acc_y;
	float acc_z;
	float gyro_x;
	float gyro_y;
	float gyro_z;
	float mag_x;
	float mag_y;
	float mag_z;
	float temp_acc;
	float temp_gyro;
	float temp_mag;
};


struct log_sens_s {
	LOG_PACKET_HEADER
	float baro_pres;
	float baro_alt;
	float baro_temp;
};

struct log_alt_s {
	LOG_PACKET_HEADER
	float pos;
	float vel;
	float baro_alt;
	float baro_vel;
	float baro_corr;
	float acc_alt;
	float acc_vel;
	float acc;
	float bias;
	float alt_ref;
	float baro;
};

#pragma pack(pop)


void log_init(void);
void log_write(void* pkt, uint16_t len);
int32_t log_read(uint32_t offset, uint8_t* data, uint16_t len);
void log_run(void);

void log_write_att(uint16_t rate);
void log_write_imu(uint16_t rate);
void log_write_sens(uint16_t rate);
void log_write_alt(uint16_t rate);

uint32_t log_get_size(void);

void log_stop(void);
bool log_need_record(void);


#define NAME_SIZE		5
#define FORMAT_SIZE		16
#define LABELS_SIZE		64

#define HEAD_BYTE1  0xA3    // Decimal 163
#define HEAD_BYTE2  0x95    // Decimal 149

