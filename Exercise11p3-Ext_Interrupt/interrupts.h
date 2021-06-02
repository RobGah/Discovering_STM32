#ifndef INTERRUPTS_H
#define INTERRUPTS_H

void config_NVIC(int IRQchannel, int IRQChannelSubPriority);
void config_EXTIO(uint8_t GPIO_PortSource,uint8_t GPIO_PinSource,uint32_t EXT_Line, 
    uint8_t EXTI_Mode,uint8_t EXTI_Trigger_Mode);

#endif