#include <stm32f10x_dac.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x.h>
#include <stm32f10x_rcc.h>

int DAC_init(uint32_t channel)
{
    uint16_t DAC_Pin;
    if(channel == DAC_Channel_1)
    {
        DAC_Pin = GPIO_Pin_4;
    }

    else if (channel == DAC_Channel_2)
    {
        DAC_Pin == GPIO_Pin_5;
    }
    
    else
    {
        return -1;
    }

    // declare data structures
    GPIO_InitTypeDef GPIO_InitStructure;
    DAC_InitTypeDef DAC_InitStructure;
    
    // enable clocks
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC , ENABLE);
    
    // Config Pins - either PA4 or PA5
    GPIO_StructInit (& GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = DAC_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; // done to disconnect the digital buffer!
    GPIO_Init(GPIOA , &GPIO_InitStructure);
    
    // DAC channel1 Configuration
    DAC_StructInit (& DAC_InitStructure);
    DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
    DAC_Init(channel , &DAC_InitStructure);
    
    // Enable DAC
    DAC_Cmd(channel , ENABLE);

    return 0;
}