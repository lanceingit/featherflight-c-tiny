#include <string.h>

#include "board.h"

#include "spi.h"
#include "spi_flash.h"

#include "timer.h"


#define DISABLE_SPI_FLASH \
    do { \
        GPIO_SetBits(GPIOB, GPIO_Pin_12); \
    } while (0)

#define ENABLE_SPI_FLASH \
    do { \
        GPIO_ResetBits(GPIOB, GPIO_Pin_12); \
    } while (0)


SpiFlash spi_flash = {
    .is_busy = false,
};

static SpiFlash* this=&spi_flash;


static int8_t spi_flash_performOneByteCommand(uint8_t command)
{
	int8_t ret;
    ENABLE_SPI_FLASH;

    ret = spi_transfer_byte(this->spi, NULL, command);

    DISABLE_SPI_FLASH;

    return ret;
}


static int8_t spi_flash_writeEnable()
{
    if(spi_flash_performOneByteCommand(M25P16_INSTRUCTION_WRITE_ENABLE) == 0)
    {
        this->is_busy = true;
        return 0;
    }
    else
    {
    	return -1;
    }
}

static void spi_flash_readStatus(uint8_t* status)
{
    uint8_t command[2] = { M25P16_INSTRUCTION_READ_STATUS_REG, 0 };
    uint8_t in[2];

    ENABLE_SPI_FLASH;

    spi_transfer(this->spi, in, command, sizeof(command));

    DISABLE_SPI_FLASH;

    *status = in[1];
}

static bool spi_flash_isReady(void)
{
    uint8_t status;
    
    spi_flash_readStatus(&status);
    
    this->is_busy = this->is_busy && ((status & M25P16_STATUS_FLAG_WRITE_IN_PROGRESS) != 0);

    return !this->is_busy;
}

//static int8_t spi_flash_waitForReady(uint32_t timeout_ms)
//{
//    times_t time = timer_now();
//    while (!spi_flash_isReady()) {
//        if (timer_now() - time > timeout_ms*1000) {
//            return -1;
//        }
//    }

//    return 0;
//}

static void spi_flash_readChipID(uint32_t* id)
{
    uint8_t out[] = { M25P16_INSTRUCTION_RDID, 0, 0, 0 };
    uint8_t in[4] = {0};

//    delay_ms(50); 

    ENABLE_SPI_FLASH;

    spi_transfer(this->spi, in, out, sizeof(out));

    DISABLE_SPI_FLASH;

    *id = (in[1] << 16) | (in[2] << 8) | (in[3]);
}

int8_t spi_flash_eraseSector(uint32_t address)
{
    uint8_t out[] = { M25P16_INSTRUCTION_SECTOR_ERASE, (uint8_t)((address >> 16) & 0xFF), (uint8_t)((address >> 8) & 0xFF), (uint8_t)(address & 0xFF)};
    int8_t ret;

    if(!spi_flash_isReady()) return -1;


    if(spi_flash_writeEnable()<0) return -1;

    ENABLE_SPI_FLASH;

    ret = spi_transfer(this->spi, NULL, out, sizeof(out));

    DISABLE_SPI_FLASH;

    return ret;
}

bool spi_flash_eraseCompletely()
{
	if(!spi_flash_isReady()) return false;

	if(spi_flash_writeEnable()<0) return false;

    if(spi_flash_performOneByteCommand(M25P16_INSTRUCTION_BULK_ERASE)==0) 
        return true;
    else 
        return false;
}

int8_t spi_flash_pageProgram(uint32_t address, const uint8_t* data, uint16_t length)
{
	int8_t ret;
    uint8_t command[] = { M25P16_INSTRUCTION_PAGE_PROGRAM, (uint8_t)((address >> 16) & 0xFF), (uint8_t)((address >> 8) & 0xFF), (uint8_t)(address & 0xFF)};

    if(!spi_flash_isReady()) return -1;

    if(spi_flash_writeEnable()<0) return -1;

    ENABLE_SPI_FLASH;

    if(spi_transfer(this->spi, NULL, command, sizeof(command)))
    {
        ret = spi_transfer(this->spi, NULL, data, length);
    }
    else
    {
    	ret = -1;
    }

    DISABLE_SPI_FLASH;

    return ret;
}

int32_t spi_flash_readBytes(uint32_t address, uint8_t* buffer, uint16_t length)
{
    uint8_t command[] = { M25P16_INSTRUCTION_READ_BYTES, (uint8_t)((address >> 16) & 0xFF), (uint8_t)((address >> 8) & 0xFF), (uint8_t)(address & 0xFF)};

    if(!spi_flash_isReady()) return -1;

    ENABLE_SPI_FLASH;

    if(spi_transfer(this->spi, NULL, command, sizeof(command))) {
        if(!spi_transfer(this->spi, buffer, NULL, length)) 
            length = 0;
    } else {
    	length = 0;
    }

    DISABLE_SPI_FLASH;

    return length;
}

//SpiFlash* spi_flash_getGeometry()
//{
//    return this;
//}

uint32_t spi_flash_get_totalSize(void)
{
    return this->totalSize;
}

uint16_t spi_flash_get_pageSize(void)
{
    return this->pageSize;
}

bool spi_flash_init()
{
	this->is_busy=false;
	this->pageSize = FLASH_PAGESIZE;

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

    DISABLE_SPI_FLASH;

    //Maximum speed for standard READ command is 20mHz, other commands tolerate 25mHz
    spi_set_divisor(this->spi, 2);

    uint32_t id;
    
    spi_flash_readChipID(&id);
    
    bool right_chip = true;
    switch (id) {
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
            right_chip = false;
            break;
    }
    
    if(right_chip) {
        this->sectorSize = this->pagesPerSector * this->pageSize;
        this->totalSize = this->sectorSize * this->sectors;
        this->is_busy = true; 
    }

    return right_chip;
}
    
    
