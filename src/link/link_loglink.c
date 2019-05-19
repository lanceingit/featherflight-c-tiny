#include "board.h"
#include "link_loglink.h"
#include "debug.h"
#include "timer.h"
#include "log.h"
#include "serial.h"
#include "mm.h"


#define RX_BUF_SIZE 100
#define TX_BUF_SIZE 5000

Serial* _logport;
uint8_t _logrxBuf[RX_BUF_SIZE];
uint8_t _logtxBuf[TX_BUF_SIZE];

LoglinkParseStep loglink_parse_step = LOGLINK_PARSE_STEP_HEAD1;
uint16_t  loglink_parse_data_count;
uint16_t loglink_parse_checksum;

uint16_t package_num = 0;

uint8_t data_buf[1024+20];
uint8_t recv_data_buf[1024+20];
loglink_message_t msg_recv;

//////////////////////////////
uint16_t loglink_crc16_init()
{
    return 0xffff;
}

uint16_t loglink_crc16_update(uint8_t data, uint16_t crc)
{
    uint8_t tmp;

    tmp = data ^ (uint8_t)(crc & 0xff);
    tmp ^= (tmp<<4);
    crc = (crc>>8) ^ (tmp<<8) ^ (tmp <<3) ^ (tmp>>4);

    return crc;
}

void loglink_send(uint8_t* data, uint16_t len)
{
    serial_write(_logport, data, len);
}

void loglink_encode(loglink_message_t* msg)
{
    uint16_t count = 0;
    uint16_t checksum = 0;
    uint8_t* buf=NULL;

    msg->head1 = LOGLINK_HEAD_1;
    msg->head2 = LOGLINK_HEAD_2;

    checksum = loglink_crc16_init();
    checksum = loglink_crc16_update(msg->length>>8, checksum);
    checksum = loglink_crc16_update(msg->length&0xFF, checksum);
    checksum = loglink_crc16_update(msg->msgid, checksum);
    for(count = 0 ; count < msg->length ; count++) {
        checksum = loglink_crc16_update(msg->data[count], checksum);
    }

    msg->checksum = checksum;

    buf = mm_malloc(msg->length+LOGLINK_MIN_LEN);
    if(buf != NULL) {
        buf[0] = msg->head1;
        buf[1] = msg->head2;
        buf[2] = msg->length>>8;
        buf[3] = msg->length&0xFF;
        buf[4] = msg->msgid;
        for(count = 0; count < msg->length ; count++) {
            buf[5+count] = msg->data[count];
        }
        buf[5+count] = (msg->checksum >> 8) & 0xFF;
        buf[6+count] = msg->checksum & 0xFF;

        loglink_send(buf, msg->length + LOGLINK_MIN_LEN);
        mm_free(buf);
    }
}


#define BSWAP_32(x) \
   (uint32_t)((((uint32_t)(x) & 0xff000000) >> 24) | \
              (((uint32_t)(x) & 0x00ff0000) >> 8) | \
              (((uint32_t)(x) & 0x0000ff00) << 8) | \
              (((uint32_t)(x) & 0x000000ff) << 24) \
             )

void loglink_msg_response_info_send(uint32_t size)
{
    loglink_message_t msg;
    loglink_response_info_s data;

    data.size = BSWAP_32(size);

    msg.msgid = LOGLINK_MSG_ID_RESPONSE_INFO;
    msg.data = (uint8_t*)&data;
    msg.length = sizeof(data);

    loglink_encode(&msg);
}

void loglink_msg_send_data_send(uint16_t package_num, uint16_t len, uint8_t* log)
{
    loglink_message_t msg;
    loglink_send_data_s* data = (loglink_send_data_s*)data_buf;

    data->package_num = package_num;
    //data->len = len;
    //uint8_t tmp
    data->len = ((len>>8)&0xFF)+((len<<8)&0xFF00);
    memcpy(data->data, log, len);

    msg.msgid = LOGLINK_MSG_ID_SEND_DATA;
    msg.data = (uint8_t*)data;
    msg.length = sizeof(*data);

    loglink_encode(&msg);
}
//////////////////////////////

void loglink_handle_message(loglink_message_t* msg)
{
    switch(msg->msgid) {
    case LOGLINK_MSG_ID_REQUEST_INFO:
        loglink_msg_response_info_send(log_get_size());
        package_num = 0;
        log_stop();
        break;
    case LOGLINK_MSG_ID_REQUEST_DATA: {
        uint16_t package_size=1024;
        uint8_t* data=NULL;
        data = mm_malloc(1024);
        if(data != NULL) {

            while(1) {

                package_size = log_get_size() - 1024*package_num;
                if(package_size > 1024) {
                    package_size = 1024;
                }
                log_read(1024*package_num, data, package_size);
                loglink_msg_send_data_send(package_num, 1024, data);
                if((log_get_size() - 1024*package_num) <= 1024)
                    break;
                package_num++;
                delay_ms(5);
            }
            mm_free(data);
        }
        break;
    }
    case LOGLINK_MSG_ID_REQUEST_END:

        break;
    default:
        break;
    }
}

bool loglink_parse_char(uint8_t ch, loglink_message_t* msg)
{
    switch(loglink_parse_step) {
    case LOGLINK_PARSE_STEP_HEAD1:
        if(ch == LOGLINK_HEAD_1) {
            loglink_parse_step = LOGLINK_PARSE_STEP_HEAD2;
        }
        break;
    case LOGLINK_PARSE_STEP_HEAD2:
        if(ch == LOGLINK_HEAD_2) {
            loglink_parse_checksum = loglink_crc16_init();
            loglink_parse_step = LOGLINK_PARSE_STEP_LEN_H;
        }
        break;
    case LOGLINK_PARSE_STEP_LEN_H:
        msg_recv.length = ch<<8;
        loglink_parse_checksum = loglink_crc16_update(ch, loglink_parse_checksum);
        loglink_parse_step = LOGLINK_PARSE_STEP_LEN_L;
        break;
    case LOGLINK_PARSE_STEP_LEN_L:
        msg_recv.length |= ch;
        loglink_parse_checksum = loglink_crc16_update(ch, loglink_parse_checksum);
        loglink_parse_step = LOGLINK_PARSE_STEP_MSG_ID;
        break;
    case LOGLINK_PARSE_STEP_MSG_ID:
        msg_recv.msgid = ch;
        loglink_parse_checksum = loglink_crc16_update(ch, loglink_parse_checksum);
        loglink_parse_data_count = 0;
        if(msg_recv.length > 0) {
            loglink_parse_step = LOGLINK_PARSE_STEP_DATA;
        }
        else {
            loglink_parse_step = LOGLINK_PARSE_STEP_CHECKSUM_H;
        }
        break;
    case LOGLINK_PARSE_STEP_DATA:
        recv_data_buf[loglink_parse_data_count] = ch;
        loglink_parse_data_count++;
        loglink_parse_checksum = loglink_crc16_update(ch, loglink_parse_checksum);
        if(loglink_parse_data_count >= msg_recv.length) {
            loglink_parse_step = LOGLINK_PARSE_STEP_CHECKSUM_H;
        }
        break;
    case LOGLINK_PARSE_STEP_CHECKSUM_H:
        if(ch == (loglink_parse_checksum >> 8)) {
            loglink_parse_step = LOGLINK_PARSE_STEP_CHECKSUM_L;
        }
        else {
            loglink_parse_step = LOGLINK_PARSE_STEP_HEAD1;
        }
        break;
    case LOGLINK_PARSE_STEP_CHECKSUM_L:
        loglink_parse_step = LOGLINK_PARSE_STEP_HEAD1;
        if(ch == (loglink_parse_checksum & 0xFF)) {
            *msg = msg_recv;
            return true;
        }
        break;
    default:
        loglink_parse_step = LOGLINK_PARSE_STEP_HEAD1;
        break;
    }

    return false;
}

bool loglink_recv(loglink_message_t* msg)
{
    uint8_t c;

    if(serial_read(_logport, &c) == 0) {
        if(loglink_parse_char(c, msg)) {
            return true;
        }
    }
    return false;
}

void loglink_init(void)
{
    _logport = serial_open(LOG_UART, 921600, _logrxBuf, RX_BUF_SIZE, _logtxBuf, TX_BUF_SIZE);
}

