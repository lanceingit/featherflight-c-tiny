#pragma once

#include "mavlink.h"

void mavlink_log_handle(mavlink_message_t* msg);
void mavlink_log_run(void);

