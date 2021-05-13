#include <misc.h>
#include "interrupts.h"
#include <stm32f10x.h>

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

