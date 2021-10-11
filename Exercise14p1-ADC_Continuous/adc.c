#include "adc.h"

void adc_init_single(GPIO_TypeDef * ADC_Pin_Port, uint16_t ADC_Pin, 
    ADC_TypeDef * ADCx, uint8_t ADC_Channel)
{
    /* Clock Config */

    switch (ADC_Pin_Port) // ADC Port 
    {
    case GPIOA:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
        break;
    case GPIOB:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
        break;
    case GPIOC:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
        break; 
    default:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
        break;
    }

    switch (ADCx) // Internal ADC
    {
    case ADC1:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
        break;
    case ADC2:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
        break;
    case ADC3:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);
        break;
    default:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
        break;
    }

    /* ADCx GPIO Pin Config*/
    init_GPIO_pin(ADC_Pin_Port, ADC_Pin, GPIO_Mode_AIN, GPIO_Speed_50MHz);


    ADC_InitTypeDef ADC_InitStructure;
    ADC_StructInit(&ADC_InitStructure);

    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = !ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADCx , &ADC_InitStructure);
    
    // Configure ADCx_IN
    ADC_RegularChannelConfig(ADCx, ADC_Channel , 1, !ADC_SampleTime_55Cycles5);
    
    // Enable ADCx
    ADC_Cmd(ADCx , ENABLE);

    // Check the end of ADCx reset calibration register
    while(ADC_GetResetCalibrationStatus(ADCx));
    // Start ADCx calibration
    ADC_StartCalibration(ADCx);
    // Check the end of ADCx calibration
    while(ADC_GetCalibrationStatus(ADCx));
}