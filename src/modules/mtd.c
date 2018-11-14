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


#define BUF_SIZE 8096

#ifdef F3_EVO
enum mtd_status
{
	MTD_ERASE,
	MTD_PROGRAM,
	MTD_PROGRAM_CONTINUE,
	MTD_IDLE,
};
static uint8_t page_buf[M25P16_PAGESIZE];
static uint32_t read_addr;
static enum mtd_status status;
static bool has_erase;
#elif LINUX
static int mtd_fd = -1;
static uint8_t page_buf[BUF_SIZE];
#endif

static struct fifo_s write_fifo;
static uint8_t buf[BUF_SIZE];

static uint32_t write_addr;

static bool full;

void mtd_shell(int argc, char *argv[]);

void mtd_init()
{
	cli_regist("mtd", mtd_shell);
	fifo_create(&write_fifo, buf, BUF_SIZE);
	write_addr = 0;
	full = false;
#ifdef F3_EVO	
	read_addr = 0;
	status = MTD_IDLE;
	has_erase = false;
#elif LINUX
	mtd_fd = open(MTD_PATH,O_RDWR | O_CREAT | O_TRUNC);
#endif	
}

void mtd_test()
{
#ifdef F3_EVO	
	if(spi_flash_eraseSector(0))
//	while(1)
	{
		for(uint16_t i=0; i<spi_flash_getGeometry()->pageSize; i++)
		{
			page_buf[i] = i;
		}

		if(spi_flash_pageProgram(0, page_buf, spi_flash_getGeometry()->pageSize))
		{
			memset(page_buf, 0, spi_flash_getGeometry()->pageSize);

			uint16_t read_len = spi_flash_readBytes(0, page_buf, spi_flash_getGeometry()->pageSize);

			while(!spi_flash_eraseSector(read_addr));
 
            memset(page_buf, 0, spi_flash_getGeometry()->pageSize);
            spi_flash_readBytes(0, page_buf, spi_flash_getGeometry()->pageSize);

		}
	}
#endif	
}

void mtd_write(uint8_t* data, uint16_t len)
{
	// PRINT("space:%u len:%u ", mtd_get_space(), len);
	if(mtd_get_space() > len) {
		for(uint16_t i=0; i<len; i++) {
			int8_t ret = fifo_write(&write_fifo, data[i]);
			if(ret < 0) {
				// PRINT("mtd buf full\n");
			}
		}
		full = false;
		// PRINT("free\n");
	} else {
		full = true;
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

uint16_t mtd_read(uint32_t offset, uint8_t* data, uint16_t len)
{
#ifdef F3_EVO	
	uint16_t read_len = spi_flash_readBytes(offset, data, len);

	read_addr += read_len;

	if(read_addr == write_addr) read_addr = 0;

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
	if(!has_erase && write_addr % spi_flash_getGeometry()->sectorSize ==0)
	{
		status = MTD_ERASE;
	}
	else if(fifo_get_count(&write_fifo) > spi_flash_getGeometry()->pageSize)
	{
		if(status != MTD_PROGRAM_CONTINUE)
		{
			status = MTD_PROGRAM;
		}
	}


	if(status == MTD_ERASE)
	{
		if(spi_flash_eraseSector(write_addr))
		{
			has_erase = true;
            status = MTD_IDLE;
//			memset(_page_buf, 0, spi_flash_getGeometry()->pageSize);
//
//			uint16_t read_len = spi_flash_readBytes(0, _page_buf, spi_flash_getGeometry()->pageSize);

		}
	}
	else if(status == MTD_PROGRAM)
	{
		for(uint16_t i=0; i<spi_flash_getGeometry()->pageSize; i++)
		{
			 fifo_read(&write_fifo, &page_buf[i]);
		}

		if(spi_flash_pageProgram(write_addr, page_buf, spi_flash_getGeometry()->pageSize))
		{
			write_addr += spi_flash_getGeometry()->pageSize;
			has_erase = false;
            status = MTD_IDLE;
            
//			memset(_page_buf, 0x5C, spi_flash_getGeometry()->pageSize);

//			uint16_t read_len = spi_flash_readBytes(0, _page_buf, spi_flash_getGeometry()->pageSize);
            

		}
		else
		{
			status = MTD_PROGRAM_CONTINUE;
		}
	}
	else if(status == MTD_PROGRAM_CONTINUE)
	{
		if(spi_flash_pageProgram(write_addr, page_buf, spi_flash_getGeometry()->pageSize))
		{
			write_addr += spi_flash_getGeometry()->pageSize;
			has_erase = false;
            status = MTD_IDLE;
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
	return spi_flash_getGeometry()->totalSize - write_addr;
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
	return full;
}

uint32_t mtd_get_store()
{
	return write_addr;
}

void mtd_shell(int argc, char *argv[])
{
	if(argc == 2) {
		if(strcmp(argv[1],"status") == 0) {
		#ifdef F3_EVO	
			PRINT("total:\t%u\n", spi_flash_getGeometry());
		#elif LINUX
			PRINT("total:\t%u\n", MTD_FILE_SIZE_MAX);
		#endif 			
			PRINT("free:\t%u\n", mtd_get_space());
			PRINT("occupy:\t%u\n", write_addr);
			PRINT("full:\t%s\n", full? "yes":"no");

			fifo_print(&write_fifo);

			return;
		}
	}
	cli_device_write("missing command: try 'status' ");
}
