#include "board.h"
#include "motor.h"

#define PWM_BRUSHED_TIMER_MHZ 24
#define BRUSHED_MOTORS_PWM_RATE 16000
#define PWM_PERIOD  ((PWM_BRUSHED_TIMER_MHZ*1000000)/BRUSHED_MOTORS_PWM_RATE)

Motor motor[4] = 
{
    {
        .tim = TIM16,
        .port = GPIOA,
        .pin = GPIO_Pin_6,
        .pin_af = GPIO_AF_1,
        .af_src = GPIO_PinSource6,
        .channel = TIM_Channel_1,
        .period = PWM_PERIOD,
    },
    {
        .tim = TIM17,
        .port = GPIOA,
        .pin = GPIO_Pin_7,
        .pin_af = GPIO_AF_1,
        .af_src = GPIO_PinSource7,
        .channel = TIM_Channel_1,
        .period = PWM_PERIOD,
    },
    {
        .tim = TIM4,
        .port = GPIOA,
        .pin = GPIO_Pin_11,
        .pin_af = GPIO_AF_10,
        .af_src = GPIO_PinSource11,
        .channel = TIM_Channel_1,
        .period = PWM_PERIOD,
    },
    {
        .tim = TIM4,
        .port = GPIOA,
        .pin = GPIO_Pin_12,
        .pin_af = GPIO_AF_10,
        .af_src = GPIO_PinSource12,
        .channel = TIM_Channel_2,
        .period = PWM_PERIOD,
    },
};

static Motor* this = motor;

//[1000,2000]
void motor_set(uint8_t num, uint16_t val)
{
    if(val < 1000) val = 1000;
    if(val > 2000) val = 2000;
    *this[num].ccr = (val - 1000) * this[num].period / 1000;
}

static void configTimeBase(TIM_TypeDef *tim, uint16_t period, uint8_t mhz)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure = {
        .TIM_ClockDivision = 0,
        .TIM_CounterMode = TIM_CounterMode_Up,
    };

    TIM_TimeBaseStructure.TIM_Period = (period - 1) & 0xffff; // AKA TIMx_ARR

    // "The counter clock frequency (CK_CNT) is equal to f CK_PSC / (PSC[15:0] + 1)." - STM32F10x Reference Manual 14.4.11
    // Thus for 1Mhz: 72000000 / 1000000 = 72, 72 - 1 = 71 = TIM_Prescaler
    TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock / ((uint32_t)mhz * 1000000)) - 1;

    TIM_TimeBaseInit(tim, &TIM_TimeBaseStructure);
}

static void pwmOCConfig(TIM_TypeDef *tim, uint8_t channel, uint16_t value)
{
    TIM_OCInitTypeDef  TIM_OCInitStructure = {
        .TIM_OCMode = TIM_OCMode_PWM2,
        .TIM_OutputState = TIM_OutputState_Enable,
        .TIM_OutputNState = TIM_OutputNState_Disable,
        .TIM_OCPolarity = TIM_OCPolarity_Low,
        .TIM_OCIdleState = TIM_OCIdleState_Set,
    };
    TIM_OCInitStructure.TIM_Pulse = value;

    switch (channel) {
    case TIM_Channel_1:
        TIM_OC1Init(tim, &TIM_OCInitStructure);
        TIM_OC1PreloadConfig(tim, TIM_OCPreload_Enable);
        break;
    case TIM_Channel_2:
        TIM_OC2Init(tim, &TIM_OCInitStructure);
        TIM_OC2PreloadConfig(tim, TIM_OCPreload_Enable);
        break;
    case TIM_Channel_3:
        TIM_OC3Init(tim, &TIM_OCInitStructure);
        TIM_OC3PreloadConfig(tim, TIM_OCPreload_Enable);
        break;
    case TIM_Channel_4:
        TIM_OC4Init(tim, &TIM_OCInitStructure);
        TIM_OC4PreloadConfig(tim, TIM_OCPreload_Enable);
        break;
    }
}

void motor_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    for(uint8_t i=0; i<4; i++) {
        //IO init
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);   //TODO:
        GPIO_PinAFConfig(this[i].port, this[i].af_src, this[i].pin_af);
        
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF,
//      GPIO_InitStructure.GPIO_Speed = 0,  //AF PP not need
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP,
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL,            
        GPIO_InitStructure.GPIO_Pin = this[i].pin,
        GPIO_Init(this[i].port, &GPIO_InitStructure);

        if(i==0) {
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16, ENABLE);   //TODO:
        } 
        else if(i==1) {
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM17, ENABLE);   //TODO:
        }
        else if(i==2 || i==3) {
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);   //TODO:
        }
        configTimeBase(this[i].tim, this[i].period, PWM_BRUSHED_TIMER_MHZ);

        pwmOCConfig(this[i].tim, this[i].channel, 0);

        TIM_CtrlPWMOutputs(this[i].tim, ENABLE);
        TIM_Cmd(this[i].tim, ENABLE);

        switch (this[i].channel) {
        case TIM_Channel_1:
            this[i].ccr = &(this[i].tim->CCR1);
            break;
        case TIM_Channel_2:
            this[i].ccr = &(this[i].tim->CCR2);
            break;
        case TIM_Channel_3:
            this[i].ccr = &(this[i].tim->CCR3);
            break;
        case TIM_Channel_4:
            this[i].ccr = &(this[i].tim->CCR4);
            break;
        }        
        *this[i].ccr = 0;
    }
}


