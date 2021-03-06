/* Hardware Timer-related operations,init and config */
#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_i2c.h>
#include <stm32f10x_tim.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "timers.h"

void pwm_init(TIM_TypeDef * TIMx, uint32_t timerperiph, uint32_t prescaler_div,
     uint32_t period,uint16_t countermode, int channel)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    // enable timer clock
    RCC_APB1PeriphClockCmd(timerperiph , ENABLE);
    // configure timer e.g. 
    // PWM frequency = 100 hz with 72 ,000 ,000 hz system clock
    // 72 ,000 ,000/720 = 100 ,000
    // 100 ,000/1000 = 100
    // "div" comments below for illustration only - all this is configurable
    //SystemCoreClock is 72MHz on the Blue Pill. 
    TIM_TimeBaseStructInit (& TIM_TimeBaseStructure);
    //above inputs yield a 100kHz clock:
    TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock / prescaler_div) - 1; // e.g. div 720
    //Above inputs give timer a period of 1000 ticks or 100Hz given a 100kHz clock
    TIM_TimeBaseStructure.TIM_Period = period - 1; // e.g. div 1000
    TIM_TimeBaseStructure.TIM_CounterMode = countermode;
    TIM_TimeBaseInit(TIMx , &TIM_TimeBaseStructure);
    // PWM1 Mode configuration: Channel2
    // Edge -aligned; not single pulse mode
    TIM_OCStructInit (& TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    
    switch(channel)
    {
        case 1:
            TIM_OC1Init(TIMx , &TIM_OCInitStructure);
        case 2:
            TIM_OC2Init(TIMx , &TIM_OCInitStructure);
        case 3:
            TIM_OC3Init(TIMx , &TIM_OCInitStructure);
        case 4: 
            TIM_OC4Init(TIMx , &TIM_OCInitStructure);
        default:
            TIM_OC1Init(TIMx,&TIM_OCInitStructure);
    }

    
    // Enable Timer
    TIM_Cmd(TIMx , ENABLE); 
}
