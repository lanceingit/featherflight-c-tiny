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


Mtd mtd = {
    .is_full = false,
#if LINUX
    .fd = -1,
#endif    
};

static Mtd* this=&mtd;

void mtd_shell(int argc, char *argv[]);


void mtd_init()
{
	cli_regist("mtd", mtd_shell);
	fifo_init(&this->write_fifo, this->write_buf, WRITE_BUF_SIZE);
	this->write_addr = 0;
	this->is_full = false;
#ifdef F3_EVO	
	this->read_addr = 0;
	this->status = MTD_IDLE;
	this->has_erase = false;
#elif LINUX
	mtd_fd = open(MTD_PATH,O_RDWR | O_CREAT | O_TRUNC);
#endif	
}

void mtd_test()
{
#ifdef F3_EVO	
	if(spi_flash_eraseSector(0) >= 0)
//	while(1)
	{
		for(uint16_t i=0; i<spi_flash_get_pageSize(); i++)
		{
			this->page_buf[i] = i;
		}

		if(spi_flash_pageProgram(0, this->page_buf, spi_flash_get_pageSize()) >= 0)
		{
			memset(this->page_buf, 0, spi_flash_get_pageSize());

			int32_t read_len = spi_flash_readBytes(0, this->page_buf, spi_flash_get_pageSize());

			while(spi_flash_eraseSector(this->read_addr) < 0);  
 
            memset(this->page_buf, 0, spi_flash_get_pageSize());
            spi_flash_readBytes(0, this->page_buf, spi_flash_get_pageSize());

		}
	}
#endif	
}

void mtd_write(uint8_t* data, uint16_t len)
{
	// PRINT("space:%u len:%u ", mtd_get_space(), len);
	if(mtd_get_space() > len) {
		for(uint16_t i=0; i<len; i++) {
			int8_t ret = fifo_write(&this->write_fifo, data[i]);
			if(ret < 0) {
				// PRINT("mtd buf full\n");
			}
		}
		this->is_full = false;
		// PRINT("free\n");
	} else {
		this->is_full = true;
		// PRINT("full\n");
	}

	// if(mtd_get_space() < 4000) {
	// 	fifo_print(&write_fifo);
	// }	
	// if(mtd_get_space() < 21000 && mtd_get_space() > 15000) {
	// 	PRINT("write");
	// 	fifo_print(&write_fifo);
	// }
}

int32_t mtd_read(uint32_t offset, uint8_t* data, uint16_t len)
{
#ifdef F3_EVO	
	int32_t read_len = spi_flash_readBytes(offset, data, len);
    
    if(read_len > 0) {
        this->read_addr += read_len;

        if(this->read_addr == this->write_addr) this->read_addr = 0;        
    }

	return read_len;
#elif LINUX	
	return read(mtd_fd, data, len);
#else 
	return 0;	
#endif	
}


void mtd_sync()
{
#ifdef F3_EVO	
	if(!this->has_erase && this->write_addr % spi_flash_get_pageSize() == 0)
	{
		this->status = MTD_ERASE;
	}
	else if(fifo_get_count(&this->write_fifo) > spi_flash_get_pageSize())
	{
		if(this->status != MTD_PROGRAM_CONTINUE)
		{
			this->status = MTD_PROGRAM;
		}
	}


	if(this->status == MTD_ERASE)
	{
		if(spi_flash_eraseSector(this->write_addr) >= 0)
		{
			this->has_erase = true;
            this->status = MTD_IDLE;
//			memset(_page_buf, 0, spi_flash_get_pageSize());
//
//			int32_t read_len = spi_flash_readBytes(0, _page_buf, spi_flash_get_pageSize());

		}
	}
	else if(this->status == MTD_PROGRAM)
	{
		for(uint16_t i=0; i<spi_flash_get_pageSize(); i++)
		{
			 fifo_read(&this->write_fifo, &this->page_buf[i]);
		}

		if(spi_flash_pageProgram(this->write_addr, this->page_buf, spi_flash_get_pageSize()) >= 0)
		{
			this->write_addr += spi_flash_get_pageSize();
			this->has_erase = false;
            this->status = MTD_IDLE;
            
//			memset(_page_buf, 0x5C, spi_flash_get_pageSize());

//			int32_t read_len = spi_flash_readBytes(0, _page_buf, spi_flash_get_pageSize());
            

		}
		else
		{
			this->status = MTD_PROGRAM_CONTINUE;
		}
	}
	else if(this->status == MTD_PROGRAM_CONTINUE)
	{
		if(spi_flash_pageProgram(this->write_addr, this->page_buf, spi_flash_get_pageSize()) >= 0)
		{
			this->write_addr += spi_flash_get_pageSize();
			this->has_erase = false;
            this->status = MTD_IDLE;
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

uint32_t mtd_get_space()
{
#ifdef F3_EVO	
	return spi_flash_get_totalSize() - this->write_addr;
#elif LINUX
	if(MTD_FILE_SIZE_MAX > write_addr) {
		return MTD_FILE_SIZE_MAX - write_addr;
	} else {
		return 0;
	}
#else 
	return 0;
#endif		
}

bool mtd_is_full()
{
	return this->is_full;
}

uint32_t mtd_get_store()
{
	return this->write_addr;
}

void mtd_shell(int argc, char *argv[])
{
	if(argc == 2) {
		if(strcmp(argv[1],"status") == 0) {
		#ifdef F3_EVO	
			PRINT("total:\t%u\n", spi_flash_get_totalSize());
		#elif LINUX
			PRINT("total:\t%u\n", MTD_FILE_SIZE_MAX);
		#endif 			
			PRINT("free:\t%u\n", mtd_get_space());
			PRINT("occupy:\t%u\n", this->write_addr);
			PRINT("full:\t%s\n", this->is_full? "yes":"no");

			fifo_print(&this->write_fifo);

			return;
		}
	}
	cli_device_write("missing command: try 'status' ");
}
