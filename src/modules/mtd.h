#pragma once



void mtd_init(void);
void mtd_test(void);
void mtd_write(uint8_t* data, uint16_t len);
uint16_t mtd_read(uint32_t offset, uint8_t* data, uint16_t len);
void mtd_sync(void);
uint32_t mtd_get_space(void);
bool mtd_is_full(void);
uint32_t mtd_get_store(void);



