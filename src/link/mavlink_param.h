#pragma once

#include "mavlink.h"


typedef enum {
    PARAM_SENDING,
    PARAM_IDLE,
} param_status;

typedef struct {
    param_status status;
    uint16_t send_index;
} MavlinkParam;


void mavlink_param_handle(mavlink_message_t* msg);
void mavlink_param_run(void);

