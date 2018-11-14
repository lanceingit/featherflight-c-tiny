#include "board.h"

#include "serial.h"


static struct serial_s serial1 = {.inited=false};


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

struct serial_s* serial_open(USART_TypeDef *USARTx, uint32_t baud, uint8_t* rxbuf, uint16_t rxbuf_size, uint8_t* txbuf, uint16_t txbuf_size)
{
    struct serial_s* s = NULL;
    USART_InitTypeDef USART_InitStructure;
    
    if (USARTx == USART1) {
        s = &serial1;
        if(s->inited) return s;
		serial_port1_init();
    } else {
        return NULL;
    }
    
    s->USARTx = USARTx;
    s->baud = baud;
    s->rxbuf_size = rxbuf_size;
    s->rxbuf = rxbuf;
    s->txbuf_size = txbuf_size;
    s->txbuf = txbuf;
    
    fifo_create(&s->rx_fifo, rxbuf, rxbuf_size);
    fifo_create(&s->tx_fifo, txbuf, txbuf_size);


    // reduce oversampling to allow for higher baud rates
    USART_OverSampling8Cmd(s->USARTx, ENABLE);
	
    USART_StructInit(&USART_InitStructure);
    USART_InitStructure.USART_BaudRate = s->baud;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(s->USARTx, &USART_InitStructure);

    USART_Cmd(s->USARTx, ENABLE);    
		
    USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);
    
    s->inited = true;

    return s;
}

void serial_write_ch(struct serial_s* s, unsigned char ch) 
{
    fifo_write_force(&s->tx_fifo, ch);

    USART_ITConfig(s->USARTx, USART_IT_TXE, ENABLE);
}

void serial_write(struct serial_s* s, unsigned char* buf, uint16_t len) 
{
    for(uint16_t i=0; i<len; i++)
    {
        fifo_write_force(&s->tx_fifo, buf[i]);
    }

    USART_ITConfig(s->USARTx, USART_IT_TXE, ENABLE);
}

bool serial_available(struct serial_s* s) 
{
	return !fifo_is_empty(&s->rx_fifo);
}

int8_t serial_read(struct serial_s* s, uint8_t* ch) 
{
    return fifo_read(&s->rx_fifo, ch);
}


//
// Interrupt handlers
//
static void serial_IRQHandler(struct serial_s* s) 
{
    uint16_t SR = s->USARTx->ISR;
	
    if (SR & USART_FLAG_RXNE) {
        fifo_write_force(&s->rx_fifo, s->USARTx->RDR);
    }

    if (SR & USART_FLAG_TXE) {
        uint8_t ch;
        if(fifo_read(&s->tx_fifo, &ch) == 0) {
            s->USARTx->TDR = ch;
        } else {   // EOT
            USART_ITConfig(s->USARTx, USART_IT_TXE, DISABLE);
        }
    }
}

void USART1_IRQHandler(void) 
{
    serial_IRQHandler(&serial1);
}

