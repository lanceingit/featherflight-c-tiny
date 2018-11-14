#pragma once

#define WWLINK_SEND_FUNC wwlink_send


void wwlink_send(uint8_t* data, uint16_t len);

#include "wwlink.h"

void wwlink_init(void);
bool wwlink_recv(wwlink_message_t* msg);
void wwlink_stream(void);

