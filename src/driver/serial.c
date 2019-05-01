#include "board.h"

#include "serial.h"

#ifdef F3_EVO
static UART_DEV uart_tab[] = 
{
    USART1,
    NULL,
    USART3,
};
#elif SM701
static UART_DEV uart_tab[] = 
{
    0,
};
#else
static UART_DEV uart_tab[] = 
{
    0,
};
#endif


Serial serial[] = 
{
    {.inited=false},
    {.inited=false},
    {.inited=false},
};


static void serial_port_init(Serial* self, uint8_t num) 
{
#ifdef F3_EVO  
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStruct;

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;            
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;        
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    
    if(num == 1) {            
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
        
        GPIO_InitStructure.GPIO_Pin = (GPIO_Pin_9 | GPIO_Pin_10);   
        
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_7);
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_7);
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
    }
    else if(num == 3) {
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
        
        GPIO_InitStructure.GPIO_Pin = (GPIO_Pin_11 | GPIO_Pin_10);   
        
        GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_7);
        GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_7);
        GPIO_Init(GPIOB, &GPIO_InitStructure);

        NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;
    }
    
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    
    NVIC_Init(&NVIC_InitStruct);        
    
    USART_InitTypeDef USART_InitStructure;

    // reduce oversampling to allow for higher baud rates
    USART_OverSampling8Cmd(self->uart, ENABLE);
    USART_StructInit(&USART_InitStructure);
    USART_InitStructure.USART_BaudRate = self->baud;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(self->uart, &USART_InitStructure);
    
    USART_Cmd(self->uart, ENABLE);    

    USART_ITConfig(self->uart, USART_IT_RXNE, ENABLE);
	USART_ITConfig(self->uart, USART_IT_TXE, ENABLE);  
#endif        
}

Serial* serial_open(uint8_t num, uint32_t baud, uint8_t* rxbuf, uint16_t rxbuf_size, uint8_t* txbuf, uint16_t txbuf_size)
{
    Serial* self = NULL;
    
    if(num == 0) return NULL;  //many mcu uart starts from 1
    if(num > sizeof(uart_tab)/sizeof(UART_DEV)) return NULL;

    uint8_t uart_index = num-1;

    self = &serial[uart_index];
    if(self->inited) return self;

    self->uart = uart_tab[uart_index];
    if(self->uart == NULL) return NULL;
    
    self->baud = baud;
    self->rxbuf_size = rxbuf_size;
    self->rxbuf = rxbuf;
    self->txbuf_size = txbuf_size;
    self->txbuf = txbuf;
    
    fifo_init(&self->rx_fifo, rxbuf, rxbuf_size);
    fifo_init(&self->tx_fifo, txbuf, txbuf_size);

    serial_port_init(self, num);  

    self->inited = true;

    return self;
}

void serial_enable_tx_int(Serial* self)
{
#ifdef F3_EVO    
    USART_ITConfig(self->uart, USART_IT_TXE, ENABLE);
#elif SM701
    AM_REGn(UART, self->uart, IER) |= AM_HAL_UART_INT_TX;
#endif   
}

void serial_disable_tx_int(Serial* self)
{
#ifdef F3_EVO    
    USART_ITConfig(self->uart, USART_IT_TXE, DISABLE);
#elif SM701
    AM_REGn(UART, self->uart, IER) &= ~AM_HAL_UART_INT_TX;
#endif   
}

bool serial_rx_int(Serial* self)
{
#ifdef F3_EVO    
    return (self->uart->ISR & USART_FLAG_RXNE);
#elif SM701
    return (AM_REGn(UART, self->uart, IES) & AM_HAL_UART_INT_RX);
#endif    
}

bool serial_tx_int(Serial* self)
{
#ifdef F3_EVO    
    return (self->uart->ISR & USART_FLAG_TXE);
#elif SM701
    return (AM_REGn(UART, self->uart, IES) & AM_HAL_UART_INT_TX);
#endif    
}

uint8_t serial_get_rx_reg(Serial* self)
{
#ifdef F3_EVO    
    return self->uart->RDR;
#elif SM701
    return (uint8_t)(AM_REGn(UART, self->uart, DR) & 0xFF);    
#endif    
}

void serial_set_tx_reg(Serial* self, uint8_t ch)
{
#ifdef F3_EVO    
    self->uart->TDR = ch;
#elif SM701
    AM_REGn(UART, self->uart , DR) = ch;
#endif    
}

void serial_write_one(Serial* self, uint8_t val) 
{
    fifo_write_force(&self->tx_fifo, val);
    serial_enable_tx_int(self);
}

void serial_write(Serial* self, uint8_t* buf, uint16_t len) 
{
    for(uint16_t i=0; i<len; i++)
    {
        fifo_write_force(&self->tx_fifo, buf[i]);    
    }

    serial_enable_tx_int(self);
}

bool serial_available(Serial* self) 
{
	return !fifo_is_empty(&self->rx_fifo);
}

int8_t serial_read(Serial* self, uint8_t* val) 
{
    return fifo_read(&self->rx_fifo, val);
}

//
// Interrupt handlers
//
static void serial_IRQHandler(Serial* self) 
{
    if(serial_rx_int(self)) {
        fifo_write_force(&self->rx_fifo, serial_get_rx_reg(self));
    }
    
    if(serial_tx_int(self)) {
        uint8_t ch;
        if(fifo_read(&self->tx_fifo, &ch) == 0) {
            serial_set_tx_reg(self, ch);
        } else {   // EOT
            serial_disable_tx_int(self);
        }
    }
}

void USART1_IRQHandler(void) 
{
    serial_IRQHandler(&serial[0]);
}

void USART3_IRQHandler(void) 
{
    serial_IRQHandler(&serial[2]);
}
