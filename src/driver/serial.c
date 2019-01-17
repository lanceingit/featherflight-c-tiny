#include "board.h"

#include "serial.h"

#ifdef F3_EVO
static UART_DEV uart_tab[] = 
{
    USART1,
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

static struct serial_s serial[] = 
{
    {.inited=false},
};


static void serial_port_init(struct serial_s* s, uint8_t num) 
{
#ifdef F3_EVO  
    if(num == 1) {
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

    USART_InitTypeDef USART_InitStructure;

    // reduce oversampling to allow for higher baud rates
    USART_OverSampling8Cmd(s->uart, ENABLE);
	
    USART_StructInit(&USART_InitStructure);
    USART_InitStructure.USART_BaudRate = s->baud;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(s->uart, &USART_InitStructure);

    USART_Cmd(s->uart, ENABLE);    
		
    USART_ITConfig(uart, USART_IT_RXNE, ENABLE);
	USART_ITConfig(uart, USART_IT_TXE, ENABLE);    
#elif SM701    
    am_hal_uart_disable(s->uart);        
    
    if(num == 1) {
        am_hal_gpio_pin_config(1, AM_HAL_PIN_1_UART0TX | AM_HAL_GPIO_PULL24K);
        am_hal_gpio_pin_config(2, AM_HAL_PIN_2_UART0RX | AM_HAL_GPIO_PULL24K);        

        am_hal_interrupt_enable(AM_HAL_INTERRUPT_UART0);   
    }
    am_hal_uart_pwrctrl_enable(s->uart);
    am_hal_uart_clock_enable(s->uart);

    am_hal_uart_config_t uart_cfg;
    uart_cfg.ui32BaudRate = s->baud;
    uart_cfg.ui32DataBits = AM_HAL_UART_DATA_BITS_8;
    uart_cfg.bTwoStopBits = false;
    uart_cfg.ui32Parity = AM_HAL_UART_PARITY_NONE;
    uart_cfg.ui32FlowCtrl = AM_HAL_UART_FLOW_CTRL_NONE;
    am_hal_uart_config(s->uart, &uart_cfg);     

    am_hal_uart_int_enable(s->uart, AM_HAL_UART_INT_TX | AM_HAL_UART_INT_RX);

    // am_hal_uart_fifo_config(s->uart, AM_HAL_UART_RX_FIFO_7_8 | AM_HAL_UART_RX_FIFO_7_8);
    am_hal_uart_enable(s->uart);


#endif    
}

struct serial_s* serial_open(uint8_t num, uint32_t baud, uint8_t* rxbuf, uint16_t rxbuf_size, uint8_t* txbuf, uint16_t txbuf_size)
{
    struct serial_s* s = NULL;

    if(num == 0) return NULL;  //many mcu uart starts from 1
    if(num > sizeof(uart_tab)/sizeof(UART_DEV)) return NULL;

    uint8_t uart_index = num-1;

    s = &serial[uart_index];
    if(s->inited) return s;

    s->uart = uart_tab[uart_index];
    s->baud = baud;
    s->rxbuf_size = rxbuf_size;
    s->rxbuf = rxbuf;
    s->txbuf_size = txbuf_size;
    s->txbuf = txbuf;
    
    fifo_create(&s->rx_fifo, rxbuf, rxbuf_size);
    fifo_create(&s->tx_fifo, txbuf, txbuf_size);

    serial_port_init(s, num);  
    
    s->inited = true;

    return s;
}

void serial_enable_tx_int(struct serial_s* s)
{
#ifdef F3_EVO    
    USART_ITConfig(s->uart, USART_IT_TXE, ENABLE);
#elif SM701
    AM_REGn(UART, s->uart, IER) |= AM_HAL_UART_INT_TX;
#endif   
}

void serial_disable_tx_int(struct serial_s* s)
{
#ifdef F3_EVO    
    USART_ITConfig(s->uart, USART_IT_TXE, DISABLE);
#elif SM701
    AM_REGn(UART, s->uart, IER) &= ~AM_HAL_UART_INT_TX;
#endif   
}

bool serial_rx_int(struct serial_s* s)
{
#ifdef F3_EVO    
    return (s->uart->ISR & USART_FLAG_RXNE);
#elif SM701
    return (AM_REGn(UART, s->uart, IES) & AM_HAL_UART_INT_RX);
#endif    
}

bool serial_tx_int(struct serial_s* s)
{
#ifdef F3_EVO    
    return (s->uart->ISR & USART_FLAG_TXE);
#elif SM701
    return (AM_REGn(UART, s->uart, IES) & AM_HAL_UART_INT_TX);
#endif    
}

uint8_t serial_get_rx_reg(struct serial_s* s)
{
#ifdef F3_EVO    
    return s->uart->RDR;
#elif SM701
    return (uint8_t)(AM_REGn(UART, s->uart, DR) & 0xFF);    
#endif    
}

void serial_set_tx_reg(struct serial_s* s, uint8_t ch)
{
#ifdef F3_EVO    
    s->USARTx->TDR = ch;
#elif SM701
    AM_REGn(UART, s->uart , DR) = ch;
#endif    
}

void serial_write_ch(struct serial_s* s, unsigned char ch) 
{
    fifo_write_force(&s->tx_fifo, ch);
    serial_enable_tx_int(s);

    if(fifo_is_empty(&s->tx_fifo)) {
        serial_enable_tx_int(s);
        serial_set_tx_reg(s, ch);
    } else {
        fifo_write_force(&s->tx_fifo, ch);
    }    
}

void serial_write(struct serial_s* s, unsigned char* buf, uint16_t len) 
{
    if(fifo_is_empty(&s->tx_fifo)) {
        serial_enable_tx_int(s);
        for(uint16_t i=0; i<len-1; i++)
        {
            fifo_write_force(&s->tx_fifo, buf[1+i]);
        }
        serial_set_tx_reg(s, buf[0]);
    } else {
        for(uint16_t i=0; i<len; i++)
        {
            fifo_write_force(&s->tx_fifo, buf[i]);
        }
    }
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
    if (serial_rx_int(s)) {
        fifo_write_force(&s->rx_fifo, serial_get_rx_reg(s));
    }

    if (serial_tx_int(s)) {
        uint8_t ch;
        if(fifo_read(&s->tx_fifo, &ch) == 0) {
            serial_set_tx_reg(s, ch);
        } else {
            serial_disable_tx_int(s);
        }
    }
}

#ifdef F3_EVO
void USART1_IRQHandler(void) 
{
    serial_IRQHandler(&serial[0]);
}
#elif SM701
void am_uart0_isr(void)
{
    serial_IRQHandler(&serial[0]);
}
#endif
