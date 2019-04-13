#include "board.h"
#include "timer.h"
#include "mathlib.h"
#include <math.h>
#include "debug.h"
#include "lpf.h"
#include "battery.h"

#define BATT_ADC_SCALE 11
#define BATT_ADC_RANGE 0xFFF
#define VBAT_RESDIVVAL_DEFAULT 10


Batt batt;
static Batt* this = &batt;
    
volatile u16 batt_adc_value;


static float battery_adc2volt(u16 adc)
{
    return adc*(3.3f*BATT_ADC_SCALE)/BATT_ADC_RANGE;    
}

void battery_update(void)
{
    u16 bat_sample = batt_adc_value;
    
    if(bat_sample > 10 && bat_sample < 0xFFF) {
        this->volt_raw = battery_adc2volt(bat_sample);  
        this->volt = lpfrc_apply(this->volt_raw_last, this->volt_raw, 0.1f);
        this->volt_raw_last = this->volt;
        //PRINT("[BATT]raw:%d volt:%d sample:%x\n", (u32)(this->volt_raw*10), (u32)(this->volt*10), bat_sample);
    }
}

void battery_init(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);    //FIXME:

    GPIO_InitStructure.GPIO_Pin = BATT_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    //GPIO_InitStructure.GPIO_Speed = 0;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(BATT_PORT, &GPIO_InitStructure);    
    
    RCC_ADCCLKConfig(RCC_ADC12PLLCLK_Div256);  // 72 MHz divided by 256 = 281.25 kHz
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ADC12, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

    DMA_DeInit(DMA2_Channel1);

    DMA_StructInit(&DMA_InitStructure);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&BATT_ADC->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&batt_adc_value;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = 1;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

    DMA_Init(DMA2_Channel1, &DMA_InitStructure);

    DMA_Cmd(DMA2_Channel1, ENABLE);

    // calibrate

    ADC_VoltageRegulatorCmd(BATT_ADC, ENABLE);
    delay_ms(10);
    ADC_SelectCalibrationMode(BATT_ADC, ADC_CalibrationMode_Single);
    ADC_StartCalibration(BATT_ADC);
    while (ADC_GetCalibrationStatus(BATT_ADC) != RESET);
    ADC_VoltageRegulatorCmd(BATT_ADC, DISABLE);

    ADC_CommonInitTypeDef ADC_CommonInitStructure;

    ADC_CommonStructInit(&ADC_CommonInitStructure);
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_CommonInitStructure.ADC_Clock = ADC_Clock_SynClkModeDiv4;
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1;
    ADC_CommonInitStructure.ADC_DMAMode = ADC_DMAMode_Circular;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = 0;
    ADC_CommonInit(BATT_ADC, &ADC_CommonInitStructure);

    ADC_StructInit(&ADC_InitStructure);

    ADC_InitStructure.ADC_ContinuousConvMode    = ADC_ContinuousConvMode_Enable;
    ADC_InitStructure.ADC_Resolution            = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ExternalTrigConvEvent = ADC_ExternalTrigConvEvent_0;
    ADC_InitStructure.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_None;
    ADC_InitStructure.ADC_DataAlign             = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_OverrunMode           = ADC_OverrunMode_Disable;
    ADC_InitStructure.ADC_AutoInjMode           = ADC_AutoInjec_Disable;
    ADC_InitStructure.ADC_NbrOfRegChannel       = 1;

    ADC_Init(BATT_ADC, &ADC_InitStructure);

    ADC_RegularChannelConfig(ADC2, ADC_Channel_1, 1, ADC_SampleTime_601Cycles5);

    ADC_Cmd(BATT_ADC, ENABLE);

    while (!ADC_GetFlagStatus(BATT_ADC, ADC_FLAG_RDY));    //FIXME: timeout

    ADC_DMAConfig(BATT_ADC, ADC_DMAMode_Circular);

    ADC_DMACmd(BATT_ADC, ENABLE);

    ADC_StartConversion(BATT_ADC);
}

