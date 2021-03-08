/* Module for Ultrasonic Rangefinder HC-SR04 module */

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
#include "setup_main.h"
#include "HC-SR04.h"

static int calc_distance_from_echo_pulse(int pw)
{
    int distance_in_cm = pw/58; //divide pulse width by 58us or 1/(speed of sound*2)

    //explicitly (so I remember where this came from):
    //speed of sound in cm = 34300 cm/s or ~29us / cm. 
    //round trip for sound (generation + echo) = ~58us/cm.
    // pw(in microseconds) / (cm/58us) yields a result in cm.   

    return distance_in_cm;
}

void US_sensor_init(void)
{
    pwm_init(TIM4,RCC_APB1Periph_TIM4,100000,100,TIM_CounterMode_Up,4); //1k clock sig
    xprintf("TIM4 CH4 initialized!\r\n");
    init_GPIO_pin(GPIOB,GPIO_Pin_9,GPIO_Mode_AF_PP,GPIO_Speed_50MHz);
    xprintf("GPIO PB9 initialized for TIM4!\r\n");
    //book says longer periods for the IC timer than the OC timer. 
    init_input_pw_capture(TIM1, RCC_APB2Periph_TIM1, 100000, 200 ,TIM_CounterMode_Up, 
        1,TIM_ICPolarity_Rising, TIM_ICSelection_DirectTI,0,false); //500hz sig
    xprintf("Rising Edge Input Capture initialized for TIM1 CH1!\r\n");
    init_input_pw_capture(TIM1, RCC_APB2Periph_TIM1, 100000,200 ,TIM_CounterMode_Up, 
        2,TIM_ICPolarity_Falling, TIM_ICSelection_IndirectTI,TIM_TS_TI1FP1,true); //500hz sig
    xprintf("Falling Edge Input Capture initialized for TIM1 CH2!\r\n");
    xprintf("Waveform pulsewidth capture mechanism initialized!\r\n");
    //Initialize TIM1 GPIO Pin
    init_GPIO_pin(GPIOA,GPIO_Pin_8,GPIO_Mode_AF_PP,GPIO_Speed_50MHz);
    xprintf("GPIO PA8 initialized for TIM1!\r\n");

    //START PWM trig signal
    //Assuming 1k trig sig, period 1ms we need 10us or .01 of trig sig period
    //therefore, pw is 10 as 10/1000 = .01 (pw value is 0-999)
    int us_pw = 10; 
    TIM_SetCompare4(TIM4,us_pw);


}

int read_US_sensor_distance_cm(void)
{
    //unclear if I need to take diff between the 2 capture registers
    //or if the coupled capture regs + reset do it for me?
    int UScapture1 = TIM_GetCapture1(TIM4);
    xprintf("Capture 1 Register reads %d.\r\n",UScapture1);
    int UScapture2 = TIM_GetCapture2(TIM4);
    xprintf("Capture 2 Register reads %d.\r\n",UScapture2);
    int UScapturediff = UScapture2 - UScapture1;
    xprintf("Capture Register diff is %d\r\n",UScapturediff);
    int dist_cm = calc_distance_from_echo_pulse(UScapture2);
    return dist_cm;

}