#pragma once

#include "mavlink.h"


typedef enum {
	LOG_READ,
	LOG_IDLE,
} log_status;

typedef struct {
    mavlink_log_data_t log_data;
    log_status status;
} MavlinkLog;


void mavlink_log_handle(mavlink_message_t* msg);
void mavlink_log_run(void);
bool mavlink_log_reading(void);

