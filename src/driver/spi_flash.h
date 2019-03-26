#pragma once


#include "spi.h"


typedef struct 
{
    Spi* spi;
    bool is_busy;
    uint16_t sectors; // Count of the number of erasable blocks on the device

    uint16_t pagesPerSector;
    uint16_t pageSize; // In bytes

    uint32_t sectorSize; // This is just pagesPerSector * pageSize

    uint32_t totalSize;  // This is just sectorSize * sectors    
} SpiFlash;


bool spi_flash_init(void);

uint16_t spi_flash_get_pageSize(void);
uint32_t spi_flash_get_sectorSize(void);
uint32_t spi_flash_get_totalSize(void);

int8_t spi_flash_eraseSector(uint32_t address);
bool spi_flash_eraseCompletely(void);

int8_t spi_flash_pageProgram(uint32_t address, const uint8_t* data, uint16_t length);

int32_t spi_flash_readBytes(uint32_t address, uint8_t* buffer, uint16_t length);

//bool spi_flash_isReady(void);
//int8_t spi_flash_waitForReady(uint32_t timeout_ms);

SpiFlash* spi_flash_getGeometry(void);


#define M25P16_INSTRUCTION_RDID             0x9F
#define M25P16_INSTRUCTION_READ_BYTES       0x03
#define M25P16_INSTRUCTION_READ_STATUS_REG  0x05
#define M25P16_INSTRUCTION_WRITE_STATUS_REG 0x01
#define M25P16_INSTRUCTION_WRITE_ENABLE     0x06
#define M25P16_INSTRUCTION_WRITE_DISABLE    0x04
#define M25P16_INSTRUCTION_PAGE_PROGRAM     0x02
#define M25P16_INSTRUCTION_SECTOR_ERASE     0x20
#define M25P16_INSTRUCTION_BULK_ERASE       0xC7

#define M25P16_STATUS_FLAG_WRITE_IN_PROGRESS 0x01
#define M25P16_STATUS_FLAG_WRITE_ENABLED     0x02

// Format is manufacturer, memory type, then capacity
#define JEDEC_ID_MICRON_M25P16         0x202015
#define JEDEC_ID_MICRON_N25Q064        0x20BA17
#define JEDEC_ID_WINBOND_W25Q64        0xEF4017
#define JEDEC_ID_MACRONIX_MX25L3206E   0xC22016
#define JEDEC_ID_MACRONIX_MX25L6406E   0xC22017
#define JEDEC_ID_MICRON_N25Q128        0x20ba18
#define JEDEC_ID_WINBOND_W25Q128       0xEF4018

