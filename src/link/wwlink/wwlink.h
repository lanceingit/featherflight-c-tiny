#pragma once

#include "board.h"

#define WWLINK_MAGIC 0xFA
#define WWLINK_IDSRC_MASTER 0
#define WWLINK_IDSRC_CLIENT 1

#define WWLINK_ITEM_SYSTEM	0
#define WWLINK_ITEM_STATUS	1
#define WWLINK_ITEM_CONTROL	2

#define PACKED __attribute__((__packed__))

typedef struct 
{
	uint8_t magic;
	uint8_t length;
	uint8_t id_src;
	uint8_t item_id;
	uint8_t subitem_id;
	uint8_t * data;
	uint16_t checksum;    
} wwlink_message_t;


#include "wwlink_crc.h"
#include "wwlink_helpers.h"
#include "wwlink_system.h"
#include "wwlink_status.h"
#include "wwlink_control.h"

