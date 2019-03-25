#pragma once

#include "board.h"

typedef struct 
{
    bool inited;
    SPI_TypeDef* SPIx;
} Spi;

Spi* spi_open(SPI_TypeDef* SPIx);
int8_t spi_transfer_byte(Spi* self, uint8_t* out, uint8_t in);
int8_t spi_transfer(Spi* self, uint8_t* out, const uint8_t* in, uint16_t len);
void spi_set_divisor(Spi* self, uint16_t divisor);


typedef enum {
	SPI_CLOCK_INITIALIZATON = 256,
	SPI_CLOCK_SLOW          = 128, //00.56250 MHz
	SPI_CLOCK_STANDARD      = 4,   //09.00000 MHz
	SPI_CLOCK_FAST          = 2,   //18.00000 MHz
	SPI_CLOCK_ULTRAFAST     = 2,   //18.00000 MHz
} SPIClockDivider_e;

