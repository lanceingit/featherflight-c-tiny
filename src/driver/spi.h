#pragma once

#include "board.h"

struct spi_s 
{
    bool inited;
#ifdef F3_EVO    
    SPI_TypeDef* SPIx;
#endif
};

#ifdef F3_EVO
struct spi_s* spi_open(SPI_TypeDef* SPIx);
#endif
int8_t spi_transfer_byte(struct spi_s* spi, uint8_t* out, uint8_t in);
int8_t spi_transfer(struct spi_s* spi, uint8_t *out, const uint8_t *in, int len);
void spi_set_divisor(struct spi_s* spi, uint16_t divisor);


