#pragma once


static inline void wwlink_encode(wwlink_message_t* msg)
{
	uint8_t count = 0;
	uint16_t checksum = 0;
	uint8_t buf[256];
		
	msg->magic = WWLINK_MAGIC;
	msg->id_src = WWLINK_IDSRC_CLIENT;

	checksum = wwlink_crc16_init();
	checksum = wwlink_crc16_update(msg->length,checksum);
	checksum = wwlink_crc16_update(msg->id_src,checksum);
	checksum = wwlink_crc16_update(msg->item_id,checksum);
	checksum = wwlink_crc16_update(msg->subitem_id,checksum);
	for(count = 0 ; count < msg->length ; count++){
		checksum = wwlink_crc16_update(msg->data[count],checksum);		
	}

	msg->checksum = checksum;

	buf[0] = msg->magic;
	buf[1] = msg->length;
	buf[2] = msg->id_src;
	buf[3] = msg->item_id;
	buf[4] = msg->subitem_id;
	for(count = 0 ; count < msg->length ; count++){
		buf[5+count] = msg->data[count];
	}
	buf[5+count] = msg->checksum & 0xFF;
	buf[6+count] = (msg->checksum >> 8) & 0xFF;

	WWLINK_SEND_FUNC(buf, msg->length + 7);
}
