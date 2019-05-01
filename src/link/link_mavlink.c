#include "board.h"

#ifdef F3_EVO
#include "serial.h"
#elif LINUX
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#endif

#include "link_mavlink.h"
#include "sensor.h"
#include "est.h"
#include "timer.h"
#include "mathlib.h"
#include "debug.h"



#define RX_BUF_SIZE 300    
#define TX_BUF_SIZE 1024    

uint8_t sendbuf[300];
#ifdef F3_EVO
Serial * _port;
#elif LINUX
#define UDP_PORT  14550
#define UDP_IP    "192.168.100.255"
static struct sockaddr_in addr;
static struct sockaddr_in bcast_addr;
static struct sockaddr_in remote_addr;
static int _fd = -1;
static int addr_len = 0;
#endif
uint8_t _rxBuf[RX_BUF_SIZE]; 
uint8_t _txBuf[TX_BUF_SIZE];

mavlink_status_t _r_mavlink_status;
mavlink_message_t msg;
mavlink_system_t mavlink_system;

void handle_log_request_list(mavlink_message_t *msg);

void mavlink_init(void)
{   
    mavlink_system.sysid = MAV_SYS;
    mavlink_system.compid = MAV_COMP;
    
#ifdef F3_EVO   
    _port = serial_open(MAVLINK_UART, 115200, _rxBuf, RX_BUF_SIZE, _txBuf, TX_BUF_SIZE);
#elif LINUX
	int flag = 0;

	if((_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket failed\n");
		exit(1);
	}

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(14556);
	addr.sin_addr.s_addr = htonl(INADDR_ANY) ;
	flag = fcntl(_fd , F_GETFL , 0);
	fcntl(_fd,F_SETFL,flag | O_NONBLOCK);

	if(bind(_fd, (struct sockaddr *)&addr, sizeof(addr))<0){
		perror("connect failed\n");
		exit(1);
	}else{
	}

	bzero(&bcast_addr, sizeof(bcast_addr));
	bcast_addr.sin_family = AF_INET;
	bcast_addr.sin_port = htons(UDP_PORT);
	bcast_addr.sin_addr.s_addr = inet_addr("192.168.100.255") ;

	addr_len = sizeof(bcast_addr);

	int broadcast_opt = 1;

	if (setsockopt(_fd, SOL_SOCKET, SO_BROADCAST, &broadcast_opt, sizeof(broadcast_opt)) < 0) {
		perror("setting broadcast permission failed");
	}    
#endif    
}    

void mavlink_msg_send(mavlink_message_t* msg)
{
    uint16_t len;
       
    len = mavlink_msg_to_send_buffer(sendbuf, msg);
#ifdef F3_EVO
    serial_write(_port, sendbuf, len);    
#elif LINUX
    sendto(_fd, sendbuf, len, 0, (struct sockaddr *)&bcast_addr, addr_len);    
#endif
}

bool mavlink_recv(mavlink_message_t* msg)
{

#ifdef F3_EVO
    uint8_t c;

    if(serial_read(_port, &c) == 0) {
        if(mavlink_parse_char(0, c, msg, &_r_mavlink_status))
        {
            return true;
        }
    }
#elif LINUX 
    int len = 0;
    uint16_t check_len = 0;
    uint8_t buffer[300] = {};
    bzero(buffer, sizeof(buffer));
    len = recvfrom(_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&remote_addr, (socklen_t*)&addr_len);

    if(len > 0) {
        char* client_ip = inet_ntoa(remote_addr.sin_addr);
        if(strcmp(client_ip,"192.168.100.1")!=0)
       	    PRINT("ip:%s port:%d len:%d\n", client_ip, remote_addr.sin_port, len);
    }
    if(len > 0) {
        for(check_len = 0; check_len < len; check_len++) {
            if(mavlink_parse_char(0, buffer[check_len], msg, &_r_mavlink_status)) {
                return true;
            }
        }
    }
#endif

	return false;
}

void mavlink_send(mavlink_channel_t chan, const uint8_t *ch, uint16_t length)
{
    if(chan == MAVLINK_COMM_0){
    #ifdef F3_EVO
        serial_write(_port, (uint8_t*)ch, length);  
        //PRINT_BUF("mav send:", ch, length);
//        PRINT("mav send(%d):", length);
//        for(uint8_t i=0; i<length; i++) { 
//            PRINT("%02x ", ch[i]);  
//        } 
//        PRINT("\n"); 
    #elif LINUX
        sendto(_fd, ch, length, 0, (struct sockaddr *)&bcast_addr, addr_len);    
    #endif
    }
}

extern float baro_vari;
extern float baro_vel;

void mavlink_stream(void)
{
    TIMER_DEF(last_heartbeat_update_time)
    if(timer_check(&last_heartbeat_update_time, 500*1000))
    {
        mavlink_msg_heartbeat_send(MAV_CH,
                                       MAV_TYPE_QUADROTOR, MAV_AUTOPILOT_PX4, 
                                       0, 
                                       0, MAV_STATE_STANDBY);  
    }
    
    TIMER_DEF(last_sen_update_time)
    if(timer_check(&last_sen_update_time, 50*1000))
    {
//        mavlink_msg_highres_imu_send(MAV_CH,
//                               timer_now(),
//                               imu->acc.x, imu->acc.y, imu->acc.z,
//                               imu->gyro.x*M_RAD_TO_DEG,imu->gyro.y*M_RAD_TO_DEG,imu->gyro.z*M_RAD_TO_DEG,
//                               compass->mag.x, compass->mag.y, compass->mag.z,
////                               baro->pressure, (alt_est_3o.heir.ref_inited? baro->altitude_smooth-alt_est_3o.heir.ref_alt:0), baro->altitude, baro->temperature,
//                               baro->pressure, (0), baro->altitude, baro->temperature,
//                               0xFFFF);

        // Vector v;
        // imu_get_acc(0, &v);
        // mavlink_msg_named_value_float_send(MAV_CH,
        //                        timer_now(),
        //                        "acc_len",
        //                        vector_length(v));

        // imu_get_gyro(0, &v);
        // mavlink_msg_named_value_float_send(MAV_CH,
        //                        timer_now(),
        //                        "gyro_len",
        //                        vector_length(v));

        // compass_get_mag(0, &v);
        // mavlink_msg_named_value_float_send(MAV_CH,
        //                        timer_now(),
        //                        "mag_len",
        //                        vector_length(v));

    //     mavlink_msg_named_value_float_send(MAV_CH,
    //                           timer_now(),
    //                           "3o_alt",
    //                           alt_est_3o.heir.alt);          
    //     mavlink_msg_named_value_float_send(MAV_CH,
    //                           timer_now(),
    //                           "3o_vel",
    //                           alt_est_3o.heir.vel);          
    //     mavlink_msg_named_value_float_send(MAV_CH,
    //                           timer_now(),
    //                           "3o_acc",
    //                           alt_est_3o.heir.acc_neu_z);          
    //     mavlink_msg_named_value_float_send(MAV_CH,
    //                           timer_now(),
    //                           "3o_acc_u",
    //                           alt_est_3o.heir.acc_neu_z - alt_est_3o.acc_corr);                                  
    //     mavlink_msg_named_value_float_send(MAV_CH,
    //                           timer_now(),
    //                           "3o_err",
    //                           alt_est_3o.alt_err);          
    //     mavlink_msg_named_value_float_send(MAV_CH,
    //                           timer_now(),
    //                           "3o_alt_c",
    //                           alt_est_3o.alt_corr);          
    //     mavlink_msg_named_value_float_send(MAV_CH,
    //                           timer_now(),
    //                           "3o_vel_c",
    //                           alt_est_3o.vel_corr);          
    //     mavlink_msg_named_value_float_send(MAV_CH,
    //                           timer_now(),
    //                           "3o_acc_c",
    //                           alt_est_3o.acc_corr);             
                             
    }
    
    TIMER_DEF(last_att_update_time)
    if(timer_check(&last_att_update_time, 100*1000))
    {
        mavlink_msg_attitude_send(MAV_CH,
                               timer_now(), 
                               EST_ROLL*M_DEG_TO_RAD,
                               EST_PITCH*M_DEG_TO_RAD,
                               EST_YAW*M_DEG_TO_RAD,
                               EST_ROLL_RATE,
                               EST_PITCH_RATE,
                               EST_YAW_RATE
                                 );

        // mavlink_msg_named_value_float_send(MAV_CH,
        //                       timer_now(),
        //                       "bias_x",
        //                       att_est_q.heir.gyro_bias.x);

        // mavlink_msg_named_value_float_send(MAV_CH,
        //                       timer_now(),
        //                       "bias_y",
        //                       att_est_q.heir.gyro_bias.y);

//        mavlink_msg_named_value_float_send(MAV_CH,
//                               timer_now(),
//                               "corr_acc_x",
//                               att_get_corr_acc_x());

//        mavlink_msg_named_value_float_send(MAV_CH,
//                               timer_now(),
//                               "corr_acc_y",
//                               att_get_corr_acc_y());

        // mavlink_msg_named_value_float_send(MAV_CH,
        //                       timer_now(),
        //                       "corr_all_x",
        //                       att_est_q.heir.corr.x);

        // mavlink_msg_named_value_float_send(MAV_CH,
        //                       timer_now(),
        //                       "corr_all_y",
        //                       att_est_q.heir.corr.y);

        // mavlink_msg_named_value_float_send(MAV_CH,
        //                       timer_now(),
        //                       "baro_vari",
        //                       baro_vari);    

        // mavlink_msg_named_value_float_send(MAV_CH,
        //                       timer_now(),
        //                       "baro_vel",
        //                       baro_vel);       

     
    }
}


void mavlink_msg_handle(mavlink_message_t* msg)
{
	switch(msg->msgid)
	{
		case MAVLINK_MSG_ID_MANUAL_CONTROL:
        {
            mavlink_manual_control_t c;
            mavlink_msg_manual_control_decode(msg, &c);
            PRINT("target:%d x:%03d y:%04d z:%04d r:%04d b:%d\n", c.target, c.x, c.y, c.z, c.r, c.buttons);
            
            break;
        }
        default:break;
    }
}

