#pragma once

#include "board.h"

typedef struct 
{
    bool inited;
    SPI_TypeDef* SPIx;
} spi_s;

spi_s* spi_open(SPI_TypeDef* SPIx);
int8_t spi_transfer_byte(spi_s* self, uint8_t* out, uint8_t in);
int8_t spi_transfer(spi_s* self, uint8_t *out, const uint8_t *in, int len);
void spi_set_divisor(spi_s* self, uint16_t divisor);


