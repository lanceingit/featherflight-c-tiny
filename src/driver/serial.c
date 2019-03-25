#include "board.h"

#include "serial.h"


static Serial serial1 = {.inited=false};


static void serial_port1_init(void) 
{
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStruct;
	
    // Enable USART1 clock
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
    GPIO_InitStructure.GPIO_Pin = (GPIO_Pin_9 | GPIO_Pin_10);   
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;            
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;        
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_7);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_7);

    NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    
    NVIC_Init(&NVIC_InitStruct);
}

Serial* serial_open(USART_TypeDef *USARTx, uint32_t baud, uint8_t* rxbuf, uint16_t rxbuf_size, uint8_t* txbuf, uint16_t txbuf_size)
{
    Serial* self = NULL;
    USART_InitTypeDef USART_InitStructure;
    
    if (USARTx == USART1) {
        self = &serial1;
        if(self->inited) return self;
		serial_port1_init();
    } else {
        return NULL;
    }
    
    self->USARTx = USARTx;
    self->baud = baud;
    self->rxbuf_size = rxbuf_size;
    self->rxbuf = rxbuf;
    self->txbuf_size = txbuf_size;
    self->txbuf = txbuf;
    
    fifo_init(&self->rx_fifo, rxbuf, rxbuf_size);
    fifo_init(&self->tx_fifo, txbuf, txbuf_size);


    // reduce oversampling to allow for higher baud rates
    USART_OverSampling8Cmd(self->USARTx, ENABLE);
	
    USART_StructInit(&USART_InitStructure);
    USART_InitStructure.USART_BaudRate = self->baud;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(self->USARTx, &USART_InitStructure);

    USART_Cmd(self->USARTx, ENABLE);    
		
    USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);
    
    self->inited = true;

    return self;
}

void serial_write_one(Serial* self, uint8_t val) 
{
    fifo_write_force(&self->tx_fifo, val);

    USART_ITConfig(self->USARTx, USART_IT_TXE, ENABLE);
}

void serial_write(Serial* self, uint8_t* buf, uint16_t len) 
{
    for(uint16_t i=0; i<len; i++)
    {
        fifo_write_force(&self->tx_fifo, buf[i]);
    }

    USART_ITConfig(self->USARTx, USART_IT_TXE, ENABLE);
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
    uint16_t SR = self->USARTx->ISR;
	
    if (SR & USART_FLAG_RXNE) {
        fifo_write_force(&self->rx_fifo, self->USARTx->RDR);
    }

    if (SR & USART_FLAG_TXE) {
        uint8_t ch;
        if(fifo_read(&self->tx_fifo, &ch) == 0) {
            self->USARTx->TDR = ch;
        } else {   // EOT
            USART_ITConfig(self->USARTx, USART_IT_TXE, DISABLE);
        }
    }
}

void USART1_IRQHandler(void) 
{
    serial_IRQHandler(&serial1);
}

