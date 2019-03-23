#pragma once


#include "board.h"
#include "fifo.h"


typedef struct
{
    bool inited;
    
    uint32_t baud;

    uint16_t txbuf_size;
    uint8_t *txbuf;
    Fifo tx_fifo;

    uint16_t rxbuf_size;
    uint8_t *rxbuf;
    Fifo rx_fifo;

    USART_TypeDef *USARTx;
} Serial;

Serial* serial_open(USART_TypeDef *USARTx, uint32_t baud, uint8_t* rxbuf, uint16_t rxbuf_size, uint8_t* txbuf, uint16_t txbuf_size);
void serial_write_ch(Serial* self, uint8_t ch);
void serial_write(Serial* self, uint8_t* buf, uint16_t len);
bool serial_available(Serial* self);
int8_t serial_read(Serial* self, uint8_t* ch);






