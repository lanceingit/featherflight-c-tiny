#pragma once

#include "mavlink.h"


typedef enum {
//	LOG_READ,
//	LOG_IDLE,
} param_status;

typedef struct {
//    mavlink_log_data_t log_data;
//    log_status status;
} MavlinkParam;


void mavlink_param_handle(mavlink_message_t* msg);
void mavlink_param_run(void);

