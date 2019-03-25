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

    uint8_t page_buf[FLASH_PAGESIZE];
    Fifo write_fifo;
    uint8_t write_buf[WRITE_BUF_SIZE];    
    uint32_t write_addr;
    bool is_full;    
} Mtd;


void mtd_init(void);
void mtd_test(void);
void mtd_write(uint8_t* data, uint16_t len);
int32_t mtd_read(uint32_t offset, uint8_t* data, uint16_t len);
void mtd_sync(void);
uint32_t mtd_get_space(void);
bool mtd_is_full(void);
uint32_t mtd_get_store(void);



