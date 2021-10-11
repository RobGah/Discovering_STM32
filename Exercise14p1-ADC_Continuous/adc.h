#include <stm32f10x_adc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x.h>
#include <stdint.h>
#include <stm32f10x_rcc.h>
#include "setup_main.h"

#ifndef ADC_H
#define ADC_H

void adc_init_single(GPIO_TypeDef * ADC_Pin_Port, uint16_t ADC_Pin, 
    ADC_TypeDef * ADCx, uint8_t ADC_Channel);

#endif