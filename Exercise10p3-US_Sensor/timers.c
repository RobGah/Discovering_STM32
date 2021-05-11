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
#include "xprintf.h"

void pwm_init(TIM_TypeDef * TIMx, uint32_t timerperiph, uint32_t prescaler_div,
     uint32_t period,uint16_t countermode, int channel)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    // enable timer clock - Note the APB#!
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
    //TIM_OCInitStructure.TIM_Pulse = 10 ; //screw it!
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

void init_input_pw_capture(TIM_TypeDef * TIMx, uint32_t timerperiph, uint32_t prescaler_div,
    uint32_t period,uint16_t countermode, uint16_t channel,uint16_t polarity, uint16_t selection)
{
    /* 
    *****README:*****

    This function is (perhaps unnessessarily) complex.
        
    In hindsight, I probably ought to have a Timer_config function and have
    separate output and input config functions. I'll roll with this for now and might re-factor later.
    
    A 2-part function (timer + input capture) to configure input capture for 2 channels of the same timer
    to measure pulse width.

    ***Function Parameters:***
    TIMx = timer to config e.g. TIM3
    timerperiph = clock to use w/ timer e.g RCC_APB2Periph_TIM1
    prescaler div = SystemCoreClock prescaler (divider)
    period = prescaled clock period (literally how many ticks - its basically another divider)
        see notes below for the prescaler / period.
    countermode = up, down, up-down counting mode selection
    channel = which timer channel you want to config e.g. 1,2, etc
        see datasheet for how the timer channels map to stm32 pins!
        config pins (not in this function) for IN_FLOATING operation to capture signals
    polarity = rising or falling edge e.g. TIM_ICPolarity_Rising
    selection = direct or indirect (don't actually know what it does tbh)
        e.g. TIM_ICSelection_IndirectTI
    input_trigger = what you want to trigger a counter reset. e.g. TIM_TS_TI1FP1

    */

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_ICInitTypeDef TIM_ICInitStructure;
    
    // enable timer clock - N.B. the APB#!
    RCC_APB2PeriphClockCmd(timerperiph , ENABLE);
   
    //init the structs
    TIM_TimeBaseStructInit (& TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock / prescaler_div) - 1;
    TIM_TimeBaseStructure.TIM_Period = period - 1; 
    TIM_TimeBaseStructure.TIM_CounterMode = countermode;
    TIM_TimeBaseInit(TIMx , &TIM_TimeBaseStructure);

    //Input Capture setup
    TIM_ICStructInit(& TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_Channel = channel;
    TIM_ICInitStructure.TIM_ICPolarity = polarity;
    TIM_ICInitStructure.TIM_ICSelection = selection;
    TIM_ICInitStructure.TIM_ICPrescaler = 0;
    TIM_ICInitStructure.TIM_ICFilter = 0;
    TIM_ICInit(TIMx , &TIM_ICInitStructure);

    // Enable Timer
    TIM_Cmd(TIMx , ENABLE); 
}

void init_input_capture_reset(TIM_TypeDef * TIMx, uint16_t input_trigger)
    {
        /* Sets up master-slave relationship between timer channels for timer reset condition */

        //configure TIM1 for slave mode w. TI1FP1 as a reset signal
        TIM_SelectInputTrigger(TIMx , input_trigger);
        TIM_SelectSlaveMode(TIMx , TIM_SlaveMode_Reset);
        TIM_SelectMasterSlaveMode(TIMx , TIM_MasterSlaveMode_Enable);
    }

