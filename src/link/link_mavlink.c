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
#include "mavlink_log.h"
#include "timer.h"
#include "mathlib.h"
#include "debug.h"

#include "sensor.h"
#include "est.h"
#include "commander.h"
#include "navigator.h"
#include "failsafe.h"


#define RX_BUF_SIZE 300
#define TX_BUF_SIZE 1024

#ifdef F3_EVO
    Serial* _port;
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

void handle_log_request_list(mavlink_message_t* msg);

bool mavlink_recv(uint8_t ch, mavlink_message_t* msg)
{

#ifdef F3_EVO
    uint8_t c;

    if(serial_read(_port, &c) == 0) {
        if(mavlink_parse_char(0, c, msg, &_r_mavlink_status)) {
            return true;
        }
    }
#elif LINUX
    int len = 0;
    uint16_t check_len = 0;
    uint8_t buffer[300] = {};
    bzero(buffer, sizeof(buffer));
    len = recvfrom(_fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&remote_addr,
                   (socklen_t*)&addr_len);

    if(len > 0) {
        char* client_ip = inet_ntoa(remote_addr.sin_addr);
        if(strcmp(client_ip, "192.168.100.1")!=0)
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

void mavlink_send(mavlink_channel_t chan, const uint8_t* ch, uint16_t length)
{
    if(chan == MAVLINK_COMM_0) {
#ifdef F3_EVO
        serial_write(_port, (uint8_t*)ch, length);
        //PRINT_BUF("mav send:", ch, length);
//        PRINT("mav send(%d):", length);
//        for(uint8_t i=0; i<length; i++) {
//            PRINT("%02x ", ch[i]);
//        }
//        PRINT("\n");
#elif LINUX
        sendto(_fd, ch, length, 0, (struct sockaddr*)&bcast_addr, addr_len);
#endif
    }
}

extern float baro_vari;
extern float baro_vel;

enum PX4_CUSTOM_MAIN_MODE {
    PX4_CUSTOM_MAIN_MODE_MANUAL = 1,
    PX4_CUSTOM_MAIN_MODE_ALTCTL,
    PX4_CUSTOM_MAIN_MODE_POSCTL,
    PX4_CUSTOM_MAIN_MODE_AUTO,
    PX4_CUSTOM_MAIN_MODE_ACRO,
    PX4_CUSTOM_MAIN_MODE_OFFBOARD,
    PX4_CUSTOM_MAIN_MODE_STABILIZED,
    PX4_CUSTOM_MAIN_MODE_RATTITUDE
};

enum PX4_CUSTOM_SUB_MODE_AUTO {
    PX4_CUSTOM_SUB_MODE_AUTO_READY = 1,
    PX4_CUSTOM_SUB_MODE_AUTO_TAKEOFF,
    PX4_CUSTOM_SUB_MODE_AUTO_LOITER,
    PX4_CUSTOM_SUB_MODE_AUTO_MISSION,
    PX4_CUSTOM_SUB_MODE_AUTO_RTL,
    PX4_CUSTOM_SUB_MODE_AUTO_LAND,
    PX4_CUSTOM_SUB_MODE_AUTO_RTGS,
    PX4_CUSTOM_SUB_MODE_AUTO_FOLLOW_TARGET
};

union px4_custom_mode {
    struct {
        uint16_t reserved;
        uint8_t main_mode;
        uint8_t sub_mode;
    };
    uint32_t data;
    float data_float;
};

void px4_get_mavlink_mode_state(uint8_t* system_status, uint8_t* base_mode, uint32_t* custom_mode)
{
    *system_status = 0;
    *base_mode = 0;
    *custom_mode = 0;

    if(system_armed()) {
        *base_mode |= MAV_MODE_FLAG_SAFETY_ARMED;
    }

    *base_mode |= MAV_MODE_FLAG_CUSTOM_MODE_ENABLED
                  | MAV_MODE_FLAG_MANUAL_INPUT_ENABLED
                  ;

    union px4_custom_mode mode;
    mode.data = 0;

    switch(navigator_get_mode()) {
    case NAV_STABILIZE:
        mode.main_mode = PX4_CUSTOM_MAIN_MODE_STABILIZED;
        break;

    case NAV_ALTHOLD:
        mode.main_mode = PX4_CUSTOM_MAIN_MODE_ALTCTL;
        break;

    case NAV_POSHOLD:
        mode.main_mode = PX4_CUSTOM_MAIN_MODE_POSCTL;
        break;

    case NAV_TAKEOFF:
        mode.main_mode = PX4_CUSTOM_MAIN_MODE_AUTO;
        mode.sub_mode = PX4_CUSTOM_SUB_MODE_AUTO_TAKEOFF;
        break;

    case NAV_LAND:
        mode.main_mode = PX4_CUSTOM_MAIN_MODE_AUTO;
        mode.sub_mode = PX4_CUSTOM_SUB_MODE_AUTO_LAND;
        break;

    case NAV_STOP:
        mode.main_mode = PX4_CUSTOM_MAIN_MODE_MANUAL;
        break;

    default:
        break;
    }

    *custom_mode = mode.data;
    *system_status = MAV_STATE_STANDBY;
}

void mavlink_stream(void)
{
    TIMER_DEF(last_heartbeat_update_time)
    if(timer_check(&last_heartbeat_update_time, 500*1000)) {
        /*
        qgc x px4
        base_mode:same as mavlink
        system_status:same as mavlink
        custom_mode:
        [31:16]:reserved
        [15:8]:main_mode
            PX4_CUSTOM_MAIN_MODE_MANUAL = 1,
            PX4_CUSTOM_MAIN_MODE_ALTCTL,
            PX4_CUSTOM_MAIN_MODE_POSCTL,
            PX4_CUSTOM_MAIN_MODE_AUTO,
            PX4_CUSTOM_MAIN_MODE_ACRO,
            PX4_CUSTOM_MAIN_MODE_OFFBOARD,
            PX4_CUSTOM_MAIN_MODE_STABILIZED,
            PX4_CUSTOM_MAIN_MODE_RATTITUDE
        [7:0]:sub_mode
            PX4_CUSTOM_SUB_MODE_AUTO_READY = 1,
            PX4_CUSTOM_SUB_MODE_AUTO_TAKEOFF,
            PX4_CUSTOM_SUB_MODE_AUTO_LOITER,
            PX4_CUSTOM_SUB_MODE_AUTO_MISSION,
            PX4_CUSTOM_SUB_MODE_AUTO_RTL,
            PX4_CUSTOM_SUB_MODE_AUTO_LAND,
            PX4_CUSTOM_SUB_MODE_AUTO_RTGS,
            PX4_CUSTOM_SUB_MODE_AUTO_FOLLOW_TARGET
        */
        uint8_t system_status; 
        uint8_t base_mode; 
        uint32_t custom_mode;
        px4_get_mavlink_mode_state(&system_status, &base_mode, &custom_mode);
        mavlink_msg_heartbeat_send(MAV_CH,
                                   MAV_TYPE_QUADROTOR, MAV_AUTOPILOT_PX4,
                                   base_mode,
                                   custom_mode, 
                                   system_status);
    }

    if(mavlink_log_reading()) return;
    
    TIMER_DEF(last_sen_update_time)
    if(timer_check(&last_sen_update_time, 10*1000)) {
        mavlink_msg_highres_imu_send(MAV_CH,
                               timer_now(),
                               imu->acc.x, imu->acc.y, imu->acc.z,
                               imu->gyro.x*M_RAD_TO_DEG,imu->gyro.y*M_RAD_TO_DEG,imu->gyro.z*M_RAD_TO_DEG,
                               compass->mag.x, compass->mag.y, compass->mag.z,
//                               baro->pressure, (alt_est_3o.heir.ref_inited? baro->altitude_smooth-alt_est_3o.heir.ref_alt:0), baro->altitude, baro->temperature,
                               baro->pressure, (0), baro->altitude, baro->temperature,
                               0xFFFF);

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
    if(timer_check(&last_att_update_time, 100*1000)) {
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
    switch(msg->msgid) {
    case MAVLINK_MSG_ID_HEARTBEAT: 
        failsafe_link_keep();
        break;
        
    case MAVLINK_MSG_ID_MANUAL_CONTROL: {
        mavlink_manual_control_t c;
        mavlink_msg_manual_control_decode(msg, &c);
        //PRINT("target:%d x:%03d y:%04d z:%04d r:%04d b:%d\n", c.target, c.x, c.y, c.z, c.r, c.buttons);

        /*
             z ^        x ^
               |          |
               ---> r     ---> y

            x,y,r: [-1000, 1000]
                z: [-1000,    0]
        */

        commander_set_roll(0, c.y/1000.0f);
        commander_set_pitch(0, c.x/1000.0f);
        commander_set_yaw(0, c.r/1000.0f);
        commander_set_thrust(0, c.z/1000.0f);
        break;
    }
    default:
        break;
    }
}

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
    flag = fcntl(_fd, F_GETFL, 0);
    fcntl(_fd, F_SETFL, flag | O_NONBLOCK);

    if(bind(_fd, (struct sockaddr*)&addr, sizeof(addr))<0) {
        perror("connect failed\n");
        exit(1);
    }
    else {
    }

    bzero(&bcast_addr, sizeof(bcast_addr));
    bcast_addr.sin_family = AF_INET;
    bcast_addr.sin_port = htons(UDP_PORT);
    bcast_addr.sin_addr.s_addr = inet_addr("192.168.100.255") ;

    addr_len = sizeof(bcast_addr);

    int broadcast_opt = 1;

    if(setsockopt(_fd, SOL_SOCKET, SO_BROADCAST, &broadcast_opt, sizeof(broadcast_opt)) < 0) {
        perror("setting broadcast permission failed");
    }
#endif
}
