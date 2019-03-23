#pragma once


typedef struct
{
    TIM_TypeDef* tim;
    GPIO_TypeDef* port;
    uint16_t pin;
    uint8_t pin_af;
    uint8_t af_src;
    uint8_t channel;
    int16_t value;
    volatile uint32_t* ccr;
    uint16_t period;
} Motor;

extern Motor motor[4];

void motor_init(void);
void motor_set(uint8_t num, uint16_t val);

