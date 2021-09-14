#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_usart.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "setup_main.h"

/*
-This file serves as a collector for start-up / universal key functions
-A work in progress!
*/

// Timer code - COMMENTED OUT FOR EXERCISE 12 and TIME TESTING!
// static __IO uint32_t TimingDelay;

// void Delay(uint32_t nTime)
// {
//     TimingDelay = nTime;
//     while(TimingDelay !=0);
// }

// void SysTick_Handler(void)
// {
//     if (TimingDelay != 0x00)
//     {
//         TimingDelay--;
//     }
// }

//initialize onboard LED (sign of life)
void init_onboard_led(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = LED_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(LED_PORT, &GPIO_InitStructure);
    GPIO_WriteBit(LED_PORT, LED_PIN, Bit_RESET);
}

void init_GPIO_pin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pinx, int GPIO_Mode, int GPIO_Speed)
/* Generic GPIO pin setup function */
{
    if (GPIOx==GPIOA) 
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    }
    else if (GPIOx==GPIOB) 
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
    }
    
    else  
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
    }

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pinx;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode;
    GPIO_Init(GPIOx, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    //GPIO_WriteBit(GPIOx, GPIO_Pinx, Bit_RESET); //set to 0 initially
}