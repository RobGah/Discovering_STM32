#include <stm32f10x_dac.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x.h>
#include <stdint.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_dma.h>
#include "dac.h"


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


int DAC_init_w_Trig(uint32_t channel, uint32_t trigger_src)
{
    // like above, but with an output trigger source.
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
    DAC_InitStructure.DAC_Trigger = trigger_src;
    DAC_Init(channel , &DAC_InitStructure);
    
    // Enable DAC
    DAC_Cmd(channel , ENABLE);

    return 0;
}


int init_dac_dma (void *outbuf, unsigned int count) 
{
// DMA1 Clock Enable
RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 , ENABLE);

// Deinit anything stale for good measure
DMA_DeInit(DMA1_Channel3);

// Create InitStructure
DMA_InitTypeDef DMA_InitStructure;
DMA_StructInit (& DMA_InitStructure);

// All the settings
DMA_InitStructure.DMA_PeripheralBaseAddr = &DAC ->DHR12R1; // peripheral is the DAC
DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; // peripheral is the destination - memory flow is base -> peripheral
DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; // do not increment peripheral's address register pointer
DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; // obvious- halfword is 16bits
DMA_InitStructure.DMA_BufferSize = count; // obvious
DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) outbuf; // our output buffer is the base
DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; // increment the base address register pointer
DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; // obvious
DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; // runs in circular mode - continuously pull from our buffer and wraparound when at last item in buffer
DMA_InitStructure.DMA_Priority = DMA_Priority_High; // high priority
DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; // not using memory to memory transfer
DMA_Init(DMA1_Channel3 , &DMA_InitStructure);


// Enable Interrupts for complete and half transfers
DMA_ITConfig(DMA1_Channel3 , DMA_IT_TC | DMA_IT_HT , ENABLE);

// Enablee DAC 
DMA_Cmd(DMA1_Channel3 , ENABLE);
//DAC_DMACmd(DAC_Channel_1 , ENABLE); //might be better placed in audio_start()
}