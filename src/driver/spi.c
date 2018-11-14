#include <string.h>

#include "board.h"

#include "spi.h"

#define SPI_TIMEOUT    1000

typedef enum {
	SPI_CLOCK_INITIALIZATON = 256,
	SPI_CLOCK_SLOW          = 128, //00.56250 MHz
	SPI_CLOCK_STANDARD      = 4,   //09.00000 MHz
	SPI_CLOCK_FAST          = 2,   //18.00000 MHz
	SPI_CLOCK_ULTRAFAST     = 2,   //18.00000 MHz
} SPIClockDivider_e;

static struct spi_s spi2 = {.inited = false};


static void spi_port2_init(void)
{
    // Enable SPI clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15; // SCK MISO MOSI 
    GPIO_Init(GPIOB, &GPIO_InitStructure);   
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_5);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_5);  
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_5);  
}

struct spi_s* spi_open(SPI_TypeDef* SPIx)
{
    struct spi_s* s = NULL;
    SPI_InitTypeDef SPI_InitStructure;
    
    if (SPIx == SPI2) {
        s = &spi2;
        if(s->inited) return s;
		spi_port2_init();
    } else {
        return NULL;
    }

    s->SPIx = SPIx;    

    // Init SPI hardware
    SPI_I2S_DeInit(SPIx);

    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;

    SPI_RxFIFOThresholdConfig(SPIx, SPI_RxFIFOThreshold_QF);

    SPI_Init(SPIx, &SPI_InitStructure);
    SPI_Cmd(SPIx, ENABLE);

    s->inited = true;

    return s;    
}


int8_t spi_transfer_byte(struct spi_s* spi, uint8_t* out, uint8_t in)
{
    uint16_t spiTimeout = SPI_TIMEOUT;

    while (SPI_I2S_GetFlagStatus(spi->SPIx, SPI_I2S_FLAG_TXE) == RESET)
        if ((spiTimeout--) == 0)
            return -1;

    SPI_SendData8(SPI2, in);

    spiTimeout = SPI_TIMEOUT;
    while (SPI_I2S_GetFlagStatus(spi->SPIx, SPI_I2S_FLAG_RXNE) == RESET)
        if ((spiTimeout--) == 0)
            return -1;

    if(out != NULL)
    {
        *out = SPI_ReceiveData8(spi->SPIx);
    }
    return 0;
}



int8_t spi_transfer(struct spi_s* spi, uint8_t *out, const uint8_t *in, int len)
{
    uint16_t spiTimeout = SPI_TIMEOUT;

    uint8_t b;
    spi->SPIx->DR;
    while (len--) {
        b = in ? *(in++) : 0xFF;
        while (SPI_I2S_GetFlagStatus(spi->SPIx, SPI_I2S_FLAG_TXE) == RESET) {
            if ((spiTimeout--) == 0)
                return -1;
        }
        SPI_SendData8(spi->SPIx, b);
        spiTimeout = 1000;
        while (SPI_I2S_GetFlagStatus(spi->SPIx, SPI_I2S_FLAG_RXNE) == RESET) {
            if ((spiTimeout--) == 0)
                return -1;
        }
        b = SPI_ReceiveData8(spi->SPIx);
        if (out)
            *(out++) = b;
    }

    return 0;
}

void spi_set_divisor(struct spi_s* spi, uint16_t divisor)
{
#define BR_CLEAR_MASK 0xFFC7

    uint16_t tempRegister;

    SPI_Cmd(spi->SPIx, DISABLE);

    tempRegister = spi->SPIx->CR1;

    switch (divisor) {
    case 2:
        tempRegister &= BR_CLEAR_MASK;
        tempRegister |= SPI_BaudRatePrescaler_2;
        break;

    case 4:
        tempRegister &= BR_CLEAR_MASK;
        tempRegister |= SPI_BaudRatePrescaler_4;
        break;

    case 8:
        tempRegister &= BR_CLEAR_MASK;
        tempRegister |= SPI_BaudRatePrescaler_8;
        break;

    case 16:
        tempRegister &= BR_CLEAR_MASK;
        tempRegister |= SPI_BaudRatePrescaler_16;
        break;

    case 32:
        tempRegister &= BR_CLEAR_MASK;
        tempRegister |= SPI_BaudRatePrescaler_32;
        break;

    case 64:
        tempRegister &= BR_CLEAR_MASK;
        tempRegister |= SPI_BaudRatePrescaler_64;
        break;

    case 128:
        tempRegister &= BR_CLEAR_MASK;
        tempRegister |= SPI_BaudRatePrescaler_128;
        break;

    case 256:
        tempRegister &= BR_CLEAR_MASK;
        tempRegister |= SPI_BaudRatePrescaler_256;
        break;
    }

    spi->SPIx->CR1 = tempRegister;

    SPI_Cmd(spi->SPIx, ENABLE);
}
