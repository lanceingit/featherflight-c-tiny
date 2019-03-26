#pragma once


#include "board.h"
#include "fifo.h"


typedef enum {
	MTD_ERASE,
	MTD_PROGRAM,
	MTD_PROGRAM_CONTINUE,
	MTD_IDLE,
}  mtd_status;


#define WRITE_BUF_SIZE 8096

typedef struct {
#ifdef F3_EVO
    uint32_t read_addr;
    mtd_status status;
    bool has_erase;    
    
#elif LINUX
    int fd;
#endif

    uint32_t start_addr;
    uint32_t end_addr;
    uint32_t write_addr;   //offset whith start addr
    Fifo write_fifo;
    uint8_t write_buf[FLASH_PAGESIZE*2];    
    uint8_t page_buf[FLASH_PAGESIZE];

    bool is_full;    
} Mtd;


void mtd_test(void);
int8_t mtd_init(Mtd* self, uint16_t sector_start, uint16_t sector_cnt);
int8_t mtd_seek(Mtd* self, uint32_t offset);
int8_t mtd_write(Mtd* self, uint8_t* data, uint16_t len);
int32_t mtd_read(Mtd* self, uint32_t offset, uint8_t* data, uint16_t len);
void mtd_sync(Mtd* self);
uint32_t mtd_get_space(Mtd* self);
bool mtd_is_full(Mtd* self);
uint32_t mtd_get_store(Mtd* self);
void mtd_print(Mtd* self);



