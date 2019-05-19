#pragma once

#define LOGLINK_MSG_ID_REQUEST_INFO     0
#define LOGLINK_MSG_ID_RESPONSE_INFO    1
#define LOGLINK_MSG_ID_REQUEST_DATA     2
#define LOGLINK_MSG_ID_SEND_DATA        3
#define LOGLINK_MSG_ID_REQUEST_END      4

#define LOGLINK_HEAD_1 'L'
#define LOGLINK_HEAD_2 'G'

#define LOGLINK_MIN_LEN    7

typedef enum {
    LOGLINK_PARSE_STEP_HEAD1 = 0,
    LOGLINK_PARSE_STEP_HEAD2,
    LOGLINK_PARSE_STEP_LEN_H,
    LOGLINK_PARSE_STEP_LEN_L,
    LOGLINK_PARSE_STEP_MSG_ID,
    LOGLINK_PARSE_STEP_DATA,
    LOGLINK_PARSE_STEP_CHECKSUM_H,
    LOGLINK_PARSE_STEP_CHECKSUM_L,
} LoglinkParseStep;

typedef struct {
    uint8_t head1;
    uint8_t head2;
    uint16_t length;
    uint8_t msgid;
    uint8_t* data;
    uint16_t checksum;
} loglink_message_t;

typedef struct {
    uint32_t size;
} loglink_response_info_s;

typedef struct {
    uint16_t package_num;
    uint16_t len;
    uint8_t data[1024];
} loglink_send_data_s;


void loglink_init(void);
bool loglink_recv(loglink_message_t* msg);
void loglink_handle_message(loglink_message_t* msg);
