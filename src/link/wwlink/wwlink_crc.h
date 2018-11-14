#pragma once

static inline uint16_t wwlink_crc16_init()
{
	return 0xffff;
}

static inline uint16_t wwlink_crc16_update(uint8_t data, uint16_t crc)
{
	uint8_t tmp;
	
	tmp = data ^ (uint8_t)(crc & 0xff);
	tmp ^= (tmp<<4);
	crc = (crc>>8) ^ (tmp<<8) ^ (tmp <<3) ^ (tmp>>4);

	return crc;
}