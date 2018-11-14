#pragma once


#include "board.h"
#include "fifo.h"


struct serial_s
{
    bool inited;
    
    uint32_t baud;

    uint16_t txbuf_size;
    uint8_t *txbuf;
    struct fifo_s tx_fifo;

    uint16_t rxbuf_size;
    uint8_t *rxbuf;
    struct fifo_s rx_fifo;

    USART_TypeDef *USARTx;
} ;

struct serial_s* serial_open(USART_TypeDef *USARTx, uint32_t baud, uint8_t* rxbuf, uint16_t rxbuf_size, uint8_t* txbuf, uint16_t txbuf_size);
void serial_write_ch(struct serial_s* s, uint8_t ch);
void serial_write(struct serial_s* s, uint8_t* buf, uint16_t len);
bool serial_available(struct serial_s* s);
int8_t serial_read(struct serial_s* s, uint8_t* ch);






