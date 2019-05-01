#pragma once


#include "board.h"
#include "fifo.h"

#ifdef F3_EVO
#define UART_DEV    USART_TypeDef*
#elif SM701
#define UART_DEV     uint8_t
#else
#define UART_DEV     uint8_t
#endif

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

    UART_DEV uart;
} Serial;

Serial* serial_open(uint8_t num, uint32_t baud, uint8_t* rxbuf, uint16_t rxbuf_size, uint8_t* txbuf, uint16_t txbuf_size);
void serial_write_one(Serial* self, uint8_t val);
void serial_write(Serial* self, uint8_t* buf, uint16_t len);
bool serial_available(Serial* self);
int8_t serial_read(Serial* self, uint8_t* val);






