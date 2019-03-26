#include "board.h"
#include <string.h>

#include "mtd.h"

#ifdef F3_EVO
#include "spi_flash.h"
#elif LINUX
#include <fcntl.h>
#include <unistd.h>
#include "timer.h"
#endif

#include "fifo.h"
#include "debug.h"


void mtd_test(void)
{
#ifdef F3_EVO	
    Mtd mtd_test;
    
    mtd_init(&mtd_test, 0, 1);
    
	if(spi_flash_eraseSector(0) >= 0)
//	while(1)
	{
		for(uint16_t i=0; i<spi_flash_get_pageSize(); i++)
		{
			mtd_test.page_buf[i] = i;
		}

		if(spi_flash_pageProgram(0, mtd_test.page_buf, spi_flash_get_pageSize()) >= 0)
		{
			memset(mtd_test.page_buf, 0, spi_flash_get_pageSize());

			int32_t read_len = spi_flash_readBytes(0, mtd_test.page_buf, spi_flash_get_pageSize());

			while(spi_flash_eraseSector(mtd_test.read_addr) < 0);  
 
            memset(mtd_test.page_buf, 0, spi_flash_get_pageSize());
            spi_flash_readBytes(0, mtd_test.page_buf, spi_flash_get_pageSize());

		}
	}
#endif	
}

int8_t mtd_init(Mtd* self, uint16_t sector_start, uint16_t sector_cnt)
{
    if(sector_start > FLASH_SECTOR_NUM) return -1;
    if(sector_start+sector_cnt > FLASH_SECTOR_NUM) return -1;
    
    self->start_addr = sector_start*FLASH_SECTORSIZE;
    self->end_addr = self->start_addr+sector_cnt*FLASH_SECTORSIZE;
 	self->write_addr = self->start_addr;

	fifo_init(&self->write_fifo, self->write_buf, WRITE_BUF_SIZE);
    
	self->is_full = false;
#ifdef F3_EVO	
	self->read_addr = self->start_addr;
	self->status = MTD_IDLE;
	self->has_erase = false;
#elif LINUX
	self->fd = open(MTD_PATH,O_RDWR | O_CREAT | O_TRUNC);
#endif	
    
    return 0;
}

int8_t mtd_seek(Mtd* self, uint32_t offset)
{
    if(self->start_addr + offset >= self->end_addr) return -1; 
    self->write_addr = self->start_addr + offset;
    
    return 0;
}

int8_t mtd_write(Mtd* self, uint8_t* data, uint16_t len)
{
	// PRINT("space:%u len:%u ", mtd_get_space(), len);
	if(mtd_get_space(self) > len) {
		for(uint16_t i=0; i<len; i++) {
			int8_t ret = fifo_write(&self->write_fifo, data[i]);
			if(ret < 0) {
				// PRINT("mtd buf full\n");
                return ret;
			}
		}
		self->is_full = false;
		// PRINT("free\n");
	} else {
		self->is_full = true;
		// PRINT("full\n");
        return -1;
	}

	// if(mtd_get_space() < 4000) {
	// 	fifo_print(&write_fifo);
	// }	
	// if(mtd_get_space() < 21000 && mtd_get_space() > 15000) {
	// 	PRINT("write");
	// 	fifo_print(&write_fifo);
	// }
    
    return 0;
}

int32_t mtd_read(Mtd* self, uint32_t offset, uint8_t* data, uint16_t len)
{
#ifdef F3_EVO	
	int32_t read_len = spi_flash_readBytes(self->start_addr + offset, data, len);
    
    if(read_len > 0) {
        self->read_addr += read_len;

        if(self->read_addr == self->write_addr) self->read_addr = self->start_addr;        
    }

	return read_len;
#elif LINUX	
	return read(mtd_fd, data, len);
#else 
	return 0;	
#endif	
}


void mtd_sync(Mtd* self)
{
#ifdef F3_EVO	
    // when addr over a sector, erase
	if(!self->has_erase && self->write_addr % FLASH_SECTORSIZE == 0)
	{
		self->status = MTD_ERASE;
	}
    // program every page
	else if(fifo_get_count(&self->write_fifo) > FLASH_PAGESIZE)
	{
		if(self->status != MTD_PROGRAM_CONTINUE)
		{
			self->status = MTD_PROGRAM;
		}
	}


	if(self->status == MTD_ERASE)
	{
		if(spi_flash_eraseSector(self->write_addr) >= 0)
		{
			self->has_erase = true;
            self->status = MTD_IDLE;
//			memset(_page_buf, 0, spi_flash_get_pageSize());
//
//			int32_t read_len = spi_flash_readBytes(0, _page_buf, spi_flash_get_pageSize());

		}
	}
	else if(self->status == MTD_PROGRAM)
	{
		for(uint16_t i=0; i<FLASH_PAGESIZE; i++)
		{
			 fifo_read(&self->write_fifo, &self->page_buf[i]);
		}

		if(spi_flash_pageProgram(self->write_addr, self->page_buf, FLASH_PAGESIZE) >= 0)
		{
			self->write_addr += FLASH_PAGESIZE;
			self->has_erase = false;
            self->status = MTD_IDLE;
            
//			memset(_page_buf, 0x5C, spi_flash_get_pageSize());

//			int32_t read_len = spi_flash_readBytes(0, _page_buf, spi_flash_get_pageSize());
            

		}
		else
		{
			self->status = MTD_PROGRAM_CONTINUE;   //flash program fail, need to program next time
		}
	}
	else if(self->status == MTD_PROGRAM_CONTINUE)
	{
		if(spi_flash_pageProgram(self->write_addr, self->page_buf, FLASH_PAGESIZE) >= 0)
		{
			self->write_addr += FLASH_PAGESIZE;
			self->has_erase = false;
            self->status = MTD_IDLE;
		}
	}
#elif LINUX
	if(mtd_fd > 0) {
		uint16_t len = fifo_get_count(&write_fifo);

		if(len > BUF_SIZE-100) {
		// if(mtd_get_space() < 200) {
		// 	PRINT("read");
		// 	fifo_print(&write_fifo);
		// }
			for(uint16_t i=0; i<len; i++) {
				fifo_read(&write_fifo, &page_buf[i]);
			}
			write_addr += write(mtd_fd, page_buf, len);
		// if(mtd_get_space() < 200) {
		// 	PRINT("read end");
		// 	fifo_print(&write_fifo);
		// }
		}

		TIMER_DEF(last_sync_time)
		if(timer_check(&last_sync_time, 500*1000)) {
			fsync(mtd_fd);
		}
	}
#endif	
}

uint32_t mtd_get_space(Mtd* self)
{
	if(self->end_addr > self->write_addr) {
		return self->end_addr - self->write_addr;
	} else {
		return 0;
	}
}

bool mtd_is_full(Mtd* self)
{
	return self->is_full;
}

uint32_t mtd_get_store(Mtd* self)
{
	return self->write_addr-self->start_addr;
}

void mtd_print(Mtd* self)
{
#ifdef F3_EVO	
    PRINT("total:\t%u\n", FLASH_SIZE);
#elif LINUX
    PRINT("total:\t%u\n", MTD_FILE_SIZE_MAX);
#endif 			
    PRINT("free:\t%u\n", mtd_get_space(self));
    PRINT("occupy:\t%u\n", self->write_addr);
    PRINT("full:\t%s\n", self->is_full? "yes":"no");

    fifo_print(&self->write_fifo);
}
