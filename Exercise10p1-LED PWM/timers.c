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

void timer_init(TIM_TypeDef * TIMx, uint32_t timerperiph, uint32_t prescaler_div,
     uint32_t period,uint16_t countermode)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    // enable timer clock
    RCC_APB1PeriphClockCmd(timerperiph , ENABLE);
    // configure timer e.g. 
    // PWM frequency = 100 hz with 24 ,000 ,000 hz system clock
    // 24 ,000 ,000/240 = 100 ,000
    // 100 ,000/1000 = 100
    // "div" comments below for illustration only - all this is configurable
    TIM_TimeBaseStructInit (& TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock / prescaler_div) - 1; // e.g. div 240
    TIM_TimeBaseStructure.TIM_Period = period - 1; // e.g. div 1000
    TIM_TimeBaseStructure.TIM_CounterMode = countermode;
    TIM_TimeBaseInit(TIMx , &TIM_TimeBaseStructure);
    // PWM1 Mode configuration: Channel2
    // Edge -aligned; not single pulse mode
    TIM_OCStructInit (& TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OC2Init(TIMx , &TIM_OCInitStructure);
    // Enable Timer
    TIM_Cmd(TIMx , ENABLE); 
}