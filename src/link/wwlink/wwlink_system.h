#pragma once

#include <string.h>
#include <stdio.h>


#define WWLINK_ITEM_SYSTEM_ACK			0
#define WWLINK_ITEM_SYSTEM_R1	        1
#define WWLINK_ITEM_SYSTEM_R2	        2
#define WWLINK_ITEM_SYSTEM_HEART		3
#define WWLINK_ITEM_SYSTEM_MSG			4

#define WWLINK_ACK_OK	0
#define WWLINK_ACK_FAIL	1

typedef struct{
	uint8_t ack;
	uint8_t item_id;
	uint8_t subitem_id;
}wwlink_system_ack_s;

typedef struct{
	uint8_t heart;
}wwlink_system_heart_s;

typedef struct{
	uint8_t level;
	uint8_t data[64];
}wwlink_system_msg_s;

static inline void wwlink_encode_system_msg(uint8_t level,uint8_t * buf)
{
	wwlink_message_t msg;
	wwlink_system_msg_s data;

	data.level = level;
	memset(data.data,0,64);
	snprintf((char *)data.data,64,(char *)buf);

	msg.item_id = WWLINK_ITEM_SYSTEM;
	msg.subitem_id = WWLINK_ITEM_SYSTEM_MSG;
	msg.data = (uint8_t *)&data;
	msg.length = sizeof(data);

	wwlink_encode(&msg);
}

static inline void wwlink_encode_system_ack(uint8_t ack,uint8_t i_id,uint8_t si_id)
{
	wwlink_message_t msg;
	wwlink_system_ack_s data;

	data.ack = ack;
	data.item_id = i_id;
	data.subitem_id = si_id;

	msg.item_id = WWLINK_ITEM_SYSTEM;
	msg.subitem_id = WWLINK_ITEM_SYSTEM_ACK;
	msg.data = (uint8_t *)&data;
	msg.length = sizeof(data);

	wwlink_encode(&msg);
}

static inline void awlink_encode_system_heart(uint8_t i){
	wwlink_message_t msg;
	wwlink_system_heart_s data;

	data.heart= i;

	msg.item_id = WWLINK_ITEM_SYSTEM;
	msg.subitem_id = WWLINK_ITEM_SYSTEM_HEART;
	msg.data = (uint8_t *)&data;
	msg.length = sizeof(data);

	wwlink_encode(&msg);
}

static inline void wwlink_decode_system_heart(wwlink_message_t* msg)
{
	
}

static inline void wwlink_sys_handle(wwlink_message_t* msg)
{
	switch(msg->subitem_id){
		case WWLINK_ITEM_SYSTEM_HEART:
			wwlink_decode_system_heart(msg);
			break;
	}
}
