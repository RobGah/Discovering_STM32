#include <misc.h>
#include "interrupts.h"
#include <stm32f10x.h>
#include <stm32f10x_exti.h>


void config_NVIC(int IRQchannel, int IRQChannelSubPriority)
{
/*
* NVIC_PriorityGroup_0 0 bits priority , 4 bits subgroup
* NVIC_PriorityGroup_1 1 bits priority , 3 bits subgroup
* NVIC_PriorityGroup_2 2 bits priority , 2 bits subgroup
* NVIC_PriorityGroup_3 3 bits priority , 1 bits subgroup
* NVIC_PriorityGroup_4 4 bits priority , 0 bits subgroup
*/
NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

NVIC_InitTypeDef NVIC_InitStructure;
NVIC_InitStructure.NVIC_IRQChannel = IRQchannel; //e.g. TIM2_IRQn
NVIC_InitStructure.NVIC_IRQChannelSubPriority = IRQChannelSubPriority;
NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //no preemption used as per author
NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
NVIC_Init(&NVIC_InitStructure);
}


void config_EXTIO(uint8_t GPIO_PortSource,uint8_t GPIO_PinSource,uint32_t EXT_Line, 
    uint8_t EXTI_Mode,uint8_t EXTI_Trigger_Mode)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    // Connect EXTI0 to PA0
    GPIO_EXTILineConfig(GPIO_PortSource , GPIO_PinSource);
    // Configure EXTI0 line // see stm32f10x_exti.h
    EXTI_InitStructure.EXTI_Line = EXT_Line;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Mode;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init (& EXTI_InitStructure);
    // Configure NVIC EXTI0_IRQn ...
}
