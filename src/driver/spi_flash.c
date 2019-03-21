#include <string.h>

#include "board.h"

#include "spi.h"
#include "spi_flash.h"

#include "timer.h"

#define M25P16_ERASE_SIZE_MIN 	4096

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



// The timeout we expect between being able to issue page program instructions
#define DEFAULT_TIMEOUT_MILLIS       6

// These take sooooo long:
#define SECTOR_ERASE_TIMEOUT_MILLIS  5000
#define BULK_ERASE_TIMEOUT_MILLIS    21000

#define DISABLE_M25P16 GPIO_SetBits(GPIOB, GPIO_Pin_12);
#define ENABLE_M25P16  GPIO_ResetBits(GPIOB, GPIO_Pin_12);

static bool couldBeBusy=false;

static int8_t spi_flash_performOneByteCommand(uint8_t command);
static int8_t spi_flash_writeEnable(void);
static uint8_t spi_flash_readStatus(void);
static int8_t spi_flash_readIdentification(void);

spi_flash_s spi_flash;

static spi_flash_s* this=&spi_flash;

/**
 * Initialize the driver, must be called before any other routines.
 *
 * Attempts to detect a connected m25p16. If found, true is returned and device capacity can be fetched with
 * m25p16_getGeometry().
 */
bool spi_flash_init()
{
	couldBeBusy=false;
	this->pageSize = M25P16_PAGESIZE;
    /* 
        if we have already detected a flash device we can simply exit 
        
        TODO: change the init param in favour of flash CFG when ParamGroups work is done
        then cs pin can be specified in hardware_revision.c or config.c (dependent on revision).
    */
    if (this->sectors) {
        return true;
    }
    
    this->spi = spi_open(SPI_FLAHS_SPI);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; // CS
    GPIO_Init(GPIOB, &GPIO_InitStructure);    

    DISABLE_M25P16;

    //Maximum speed for standard READ command is 20mHz, other commands tolerate 25mHz
    spi_set_divisor(this->spi, 2);

    if(spi_flash_readIdentification() == 0) {
        return true;
    } else {
        return false;
    }
}

/**
 * Send the given command byte to the device.
 */
int8_t spi_flash_performOneByteCommand(uint8_t command)
{
	int8_t ret;
    ENABLE_M25P16;

    ret = spi_transfer_byte(this->spi, NULL, command);

    DISABLE_M25P16;

    return ret;
}

/**
 * The flash requires this write enable command to be sent before commands that would cause
 * a write like program and erase.
 */
int8_t spi_flash_writeEnable()
{
    if(spi_flash_performOneByteCommand(M25P16_INSTRUCTION_WRITE_ENABLE) == 0)
    {
        // Assume that we're about to do some writing, so the device is just about to become busy
        couldBeBusy = true;
        return 0;
    }
    else
    {
    	return -1;
    }
}

uint8_t spi_flash_readStatus()
{
    uint8_t command[2] = { M25P16_INSTRUCTION_READ_STATUS_REG, 0 };
    uint8_t in[2];

    ENABLE_M25P16;

    spi_transfer(this->spi, in, command, sizeof(command));

    DISABLE_M25P16;

    return in[1];
}

bool spi_flash_isReady()
{
    // If couldBeBusy is false, don't bother to poll the flash chip for its status
    couldBeBusy = couldBeBusy && ((spi_flash_readStatus() & M25P16_STATUS_FLAG_WRITE_IN_PROGRESS) != 0);

    return !couldBeBusy;
}

bool spi_flash_waitForReady(uint32_t timeoutMillis)
{
    times_t time = timer_now();
    while (!spi_flash_isReady()) {
        if (timer_now() - time > timeoutMillis*1000) {
            return false;
        }
    }

    return true;
}

/**
 * Read chip identification and geometry information (into global `geometry`).
 *
 * Returns true if we get valid ident, false if something bad happened like there is no M25P16.
 */
int8_t spi_flash_readIdentification()
{
    uint8_t out[] = { M25P16_INSTRUCTION_RDID, 0, 0, 0 };
    uint8_t in[4];
    uint32_t chipID;

    delay_ms(50); // short delay required after initialisation of SPI device instance.

    /* Just in case transfer fails and writes nothing, so we don't try to verify the ID against random garbage
     * from the stack:
     */
    in[1] = 0;

    ENABLE_M25P16;

    spi_transfer(this->spi, in, out, sizeof(out));

    // Clearing the CS bit terminates the command early so we don't have to read the chip UID:
    DISABLE_M25P16;

    // Manufacturer, memory type, and capacity
    chipID = (in[1] << 16) | (in[2] << 8) | (in[3]);

    // All supported chips use the same pagesize of 256 bytes

    switch (chipID) {
        case JEDEC_ID_MICRON_M25P16:
            this->sectors = 32;
            this->pagesPerSector = 256;
        break;
        case JEDEC_ID_MACRONIX_MX25L3206E:
            this->sectors = 64;
            this->pagesPerSector = 256;
        break;
        case JEDEC_ID_MICRON_N25Q064:
        case JEDEC_ID_WINBOND_W25Q64:
        case JEDEC_ID_MACRONIX_MX25L6406E:
            this->sectors = 128*16;
            this->pagesPerSector = 16;
        break;
        case JEDEC_ID_MICRON_N25Q128:
        case JEDEC_ID_WINBOND_W25Q128:
            this->sectors = 256;
            this->pagesPerSector = 256;
        break;
        default:
            // Unsupported chip or not an SPI NOR flash
            this->sectors = 0;
            this->pagesPerSector = 0;

            this->sectorSize = 0;
            this->totalSize = 0;
            return -1;
    }

    this->sectorSize = this->pagesPerSector * this->pageSize;
    this->totalSize = this->sectorSize * this->sectors;

    couldBeBusy = true; // Just for luck we'll assume the chip could be busy even though it isn't specced to be

    return 0;
}



/**
 * Erase a sector full of bytes to all 1's at the given byte offset in the flash chip.
 */
bool spi_flash_eraseSector(uint32_t address)
{
    uint8_t out[] = { M25P16_INSTRUCTION_SECTOR_ERASE, (uint8_t)((address >> 16) & 0xFF), (uint8_t)((address >> 8) & 0xFF), (uint8_t)(address & 0xFF)};
    bool ret;
    //waitForReady(SECTOR_ERASE_TIMEOUT_MILLIS);

    if(!spi_flash_isReady()) return false;


    if(spi_flash_writeEnable()<0) return false;

    ENABLE_M25P16;

    ret = spi_transfer(this->spi, NULL, out, sizeof(out));

    DISABLE_M25P16;

    return ret;
}

bool spi_flash_eraseCompletely()
{
    //waitForReady(BULK_ERASE_TIMEOUT_MILLIS);
	if(!spi_flash_isReady()) return false;

	if(spi_flash_writeEnable()<0) return false;

    if(spi_flash_performOneByteCommand(M25P16_INSTRUCTION_BULK_ERASE)==0) return true;
    else return false;
}

bool spi_flash_pageProgramBegin(uint32_t address)
{
    uint8_t command[] = { M25P16_INSTRUCTION_PAGE_PROGRAM, (uint8_t)((address >> 16) & 0xFF), (uint8_t)((address >> 8) & 0xFF), (uint8_t)(address & 0xFF)};

    //waitForReady(DEFAULT_TIMEOUT_MILLIS);
    if(!spi_flash_isReady()) return false;

    if(spi_flash_writeEnable()<0) return false;

    ENABLE_M25P16;

    if(spi_transfer(this->spi, NULL, command, sizeof(command))) return true;
    else return false;
}

bool spi_flash_pageProgramContinue(const uint8_t *data, int length)
{
    return spi_transfer(this->spi, NULL, data, length);
}

void spi_flash_pageProgramFinish()
{
    DISABLE_M25P16;
}

/**
 * Write bytes to a flash page. Address must not cross a page boundary.
 *
 * Bits can only be set to zero, not from zero back to one again. In order to set bits to 1, use the erase command.
 *
 * Length must be smaller than the page size.
 *
 * This will wait for the flash to become ready before writing begins.
 *
 * Datasheet indicates typical programming time is 0.8ms for 256 bytes, 0.2ms for 64 bytes, 0.05ms for 16 bytes.
 * (Although the maximum possible write time is noted as 5ms).
 *
 * If you want to write multiple buffers (whose sum of sizes is still not more than the page size) then you can
 * break this operation up into one beginProgram call, one or more continueProgram calls, and one finishProgram call.
 */
bool spi_flash_pageProgram(uint32_t address, const uint8_t *data, int length)
{
	bool ret;
    if(spi_flash_pageProgramBegin(address))
    {
        ret = spi_flash_pageProgramContinue(data, length);
    }
    else
    {
    	ret = false;
    }

    spi_flash_pageProgramFinish();

    return ret;
}

/**
 * Read `length` bytes into the provided `buffer` from the flash starting from the given `address` (which need not lie
 * on a page boundary).
 *
 * Waits up to DEFAULT_TIMEOUT_MILLIS milliseconds for the flash to become ready before reading.
 *
 * The number of bytes actually read is returned, which can be zero if an error or timeout occurred.
 */
int spi_flash_readBytes(uint32_t address, uint8_t *buffer, int length)
{
    uint8_t command[] = { M25P16_INSTRUCTION_READ_BYTES, (uint8_t)((address >> 16) & 0xFF), (uint8_t)((address >> 8) & 0xFF), (uint8_t)(address & 0xFF)};

 //    if (!waitForReady(DEFAULT_TIMEOUT_MILLIS)) {
//        return 0;
//    }
    if(!spi_flash_isReady()) return 0;

    ENABLE_M25P16;

    if(spi_transfer(this->spi, NULL, command, sizeof(command)))
    {
        if(!spi_transfer(this->spi, buffer, NULL, length)) length = 0;
    }
    else
    {
    	length = 0;
    }

    DISABLE_M25P16;

    return length;
}

/**
 * Fetch information about the detected flash chip layout.
 *
 * Can be called before calling m25p16_init() (the result would have totalSize = 0).
 */
spi_flash_s* spi_flash_getGeometry()
{
    return this;
}

