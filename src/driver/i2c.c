#include "board.h"


#include "i2c.h"

#define I2C_SHORT_TIMEOUT   ((uint32_t)0x1000)
#define I2C_LONG_TIMEOUT    ((uint32_t)(10 * I2C_SHORT_TIMEOUT))

//static uint32_t i2cTimeout;



static struct i2c_s i2c1 = {.inited = false};

static void i2c_port1_init(void) 
{
#ifdef F3_EVO 
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    RCC_I2CCLKConfig(RCC_I2C1CLK_SYSCLK);   
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; // SCL
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7; // SDA
    GPIO_Init(GPIOB, &GPIO_InitStructure);    
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_4);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_4);    
#endif    
}

#ifdef F3_EVO
struct i2c_s* i2c_open(I2C_TypeDef* I2Cx)
{
    struct i2c_s* s = NULL;
    I2C_InitTypeDef I2C_InitStructure;
    
    if (I2Cx == I2C1) {
        s = &i2c1;
        if(s->inited) return s;
		i2c_port1_init();
    } else {
        return NULL;
    }

    s->I2Cx = I2Cx;

    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C,
    I2C_InitStructure.I2C_AnalogFilter = I2C_AnalogFilter_Enable,
    I2C_InitStructure.I2C_DigitalFilter = 0x00,
    I2C_InitStructure.I2C_OwnAddress1 = 0x00,
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable,
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit,
    I2C_InitStructure.I2C_Timing = (0x00E0257A);
    I2C_Init(I2Cx, &I2C_InitStructure);

    I2C_StretchClockCmd(I2Cx, ENABLE);
    I2C_Cmd(I2Cx, ENABLE);

    s->inited = true;

    return s;
}
#endif

int8_t i2c_write(struct i2c_s* i2c, uint8_t addr, uint8_t reg, uint8_t data)
{
#ifdef F3_EVO    
    addr <<= 1;

    /* Test on BUSY Flag */
    uint32_t timeout = I2C_LONG_TIMEOUT;
    while (I2C_GetFlagStatus(i2c->I2Cx, I2C_ISR_BUSY) != RESET) {
        if ((timeout--) == 0) {
            return false;
        }
    }

    /* Configure slave address, nbytes, reload, end mode and start or stop generation */
    I2C_TransferHandling(i2c->I2Cx, addr, 1, I2C_Reload_Mode, I2C_Generate_Start_Write);

    /* Wait until TXIS flag is set */
    timeout = I2C_LONG_TIMEOUT;
    while (I2C_GetFlagStatus(i2c->I2Cx, I2C_ISR_TXIS) == RESET) {
        if ((timeout--) == 0) {
            return -1;
        }
    }

    /* Send Register address */
    I2C_SendData(i2c->I2Cx, (uint8_t) reg);

    /* Wait until TCR flag is set */
    timeout = I2C_LONG_TIMEOUT;
    while (I2C_GetFlagStatus(i2c->I2Cx, I2C_ISR_TCR) == RESET)
    {
        if ((timeout--) == 0) {
            return -1;
        }
    }

    /* Configure slave address, nbytes, reload, end mode and start or stop generation */
    I2C_TransferHandling(i2c->I2Cx, addr, 1, I2C_AutoEnd_Mode, I2C_No_StartStop);

    /* Wait until TXIS flag is set */
    timeout = I2C_LONG_TIMEOUT;
    while (I2C_GetFlagStatus(i2c->I2Cx, I2C_ISR_TXIS) == RESET) {
        if ((timeout--) == 0) {
            return -1;
        }
    }

    /* Write data to TXDR */
    I2C_SendData(i2c->I2Cx, data);

    /* Wait until STOPF flag is set */
    timeout = I2C_LONG_TIMEOUT;
    while (I2C_GetFlagStatus(i2c->I2Cx, I2C_ISR_STOPF) == RESET) {
        if ((timeout--) == 0) {
            return -1;
        }
    }

    /* Clear STOPF flag */
    I2C_ClearFlag(i2c->I2Cx, I2C_ICR_STOPCF);

    return 0;
#endif    
}

int8_t i2c_read(struct i2c_s* i2c, uint8_t addr_, uint8_t reg, uint8_t len, uint8_t* buf)
{
#ifdef F3_EVO    
    addr_ <<= 1;

    /* Test on BUSY Flag */
    uint32_t timeout = I2C_LONG_TIMEOUT;
    while (I2C_GetFlagStatus(i2c->I2Cx, I2C_ISR_BUSY) != RESET) {
        if ((timeout--) == 0) {
            return false;
        }
    }

    /* Configure slave address, nbytes, reload, end mode and start or stop generation */
    I2C_TransferHandling(i2c->I2Cx, addr_, 1, I2C_SoftEnd_Mode, I2C_Generate_Start_Write);

    /* Wait until TXIS flag is set */
    timeout = I2C_LONG_TIMEOUT;
    while (I2C_GetFlagStatus(i2c->I2Cx, I2C_ISR_TXIS) == RESET) {
        if ((timeout--) == 0) {
            return false;
        }
    }

    /* Send Register address */
    I2C_SendData(i2c->I2Cx, (uint8_t) reg);

    /* Wait until TC flag is set */
    timeout = I2C_LONG_TIMEOUT;
    while (I2C_GetFlagStatus(i2c->I2Cx, I2C_ISR_TC) == RESET) {
        if ((timeout--) == 0) {
            return false;
        }
    }

    /* Configure slave address, nbytes, reload, end mode and start or stop generation */
    I2C_TransferHandling(i2c->I2Cx, addr_, len, I2C_AutoEnd_Mode, I2C_Generate_Start_Read);

    /* Wait until all data are received */
    while (len) {
        /* Wait until RXNE flag is set */
        timeout = I2C_LONG_TIMEOUT;
        while (I2C_GetFlagStatus(i2c->I2Cx, I2C_ISR_RXNE) == RESET) {
            if ((timeout--) == 0) {
                return false;
            }
        }

        /* Read data from RXDR */
        *buf = I2C_ReceiveData(i2c->I2Cx);
        /* Point to the next location where the byte read will be saved */
        buf++;

        /* Decrement the read bytes counter */
        len--;
    }

    /* Wait until STOPF flag is set */
    timeout = I2C_LONG_TIMEOUT;
    while (I2C_GetFlagStatus(i2c->I2Cx, I2C_ISR_STOPF) == RESET) {
        if ((timeout--) == 0) {
            return false;
        }
    }

    /* Clear STOPF flag */
    I2C_ClearFlag(i2c->I2Cx, I2C_ICR_STOPCF);

    /* If all operations OK */
    return true;
#endif    
}










