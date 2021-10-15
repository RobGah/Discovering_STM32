#include "adc.h"
#include "xprintf.h"

void adc_init_single(GPIO_TypeDef * ADC_Pin_Port, uint16_t ADC_Pin, 
    ADC_TypeDef * ADCx, uint8_t ADC_Channel)
{
    /* Clock Config */

    if(ADC_Pin_Port==GPIOA) // ADC Port 
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
        xprintf("GPIOA clk enabled\r\n");
    }
    else if(ADC_Pin_Port==GPIOB)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    }
    else if(ADC_Pin_Port == GPIOC)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    }
    else
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    }

    if(ADCx==ADC1) // Internal ADC
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
        xprintf("ADC1 clk enabled \r\n");
    }
    else if(ADCx == ADC2)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
    }
    else if (ADCx== ADC3)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);
    }
    else
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    }

    /* Insurance Policy */
    ADC_DeInit(ADCx);

    ADC_InitTypeDef ADC_InitStructure;
    ADC_StructInit(&ADC_InitStructure);

    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADCx , &ADC_InitStructure);
    
    // Enable ADCx
    ADC_Cmd(ADCx , ENABLE);

    // Configure ADCx_IN
    ADC_RegularChannelConfig(ADCx, ADC_Channel , 1, ADC_SampleTime_55Cycles5);
    

    // Check the end of ADCx reset calibration register
    while(ADC_GetResetCalibrationStatus(ADCx));
    // Start ADCx calibration
    ADC_StartCalibration(ADCx);
    // Check the end of ADCx calibration
    while(ADC_GetCalibrationStatus(ADCx));

    /* ADCx GPIO Pin Config*/
    init_GPIO_pin(ADC_Pin_Port, ADC_Pin, GPIO_Mode_AIN, GPIO_Speed_2MHz);
}
