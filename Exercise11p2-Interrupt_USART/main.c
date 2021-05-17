#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_i2c.h>
#include <stm32f10x_tim.h>
#include <misc.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "uart.h"
#include "spi.h"
#include "setup_main.h"
#include "xprintf.h"
#include "timers.h"
#include "interrupts.h"

/*

Setup:

No connections. Just blinking an LED

BluePill    BluePill Pin    Note
LED         PC13             Onboard LED 


To test:
- Blink the LED at 1Hz (.5s on, .5s off) using interrupts 

For UART Debug, I'm using:

UART    BluePill    BluePill Pin 
TXD     RXD         A9
RXD     TXD         A10
GND     GND         GND
5V      5V          5V

*/

#define USE_FULL_ASSERT

/*SELECT A TEST by commenting / uncommenting these defs*/
#define BLINK_TEST

/****xprintf support****/
void myputchar(unsigned char c)
{
    uart_putc(c, USART1);
}
unsigned char mygetchar ()
{
    return uart_getc(USART1);
}

bool ledval = false;    


int main()
{
    // Configure SysTick Timer
    if(SysTick_Config(SystemCoreClock/1000))
    {
        while(1);
    }

    //setup xprintf 
    xdev_in(mygetchar); 
    xdev_out(myputchar);
    
    //uart port opened for debugging
    uart_open(USART1,9600);
    xprintf("UART is Live.\r\n");

    

    //init timers and pins for US sensor
    #ifdef BLINK_TEST
    //TIM2_IRQn shows up as typo but its legit 
    //preprocessor flag defines board and interrupts in makefile.common
        config_NVIC(TIM2_IRQn,3); 
        //1us ticks, period of 500k us = 2hz signal.
        timer_init(TIM2,RCC_APB1Periph_TIM2,100000,500000,TIM_CounterMode_Up);
        // start LED
        init_onboard_led();
        
        // Enable Timer Interrupt, enable timermingw
        TIM_ITConfig(TIM2 , TIM_IT_Update , ENABLE);
        TIM_Cmd(TIM2 , ENABLE);
    #endif

    //MAIN LOOP
    #ifdef BLINK_TEST
        while (1) { /* do nothing*/ }
    #endif
   return(0);
}

void TIM2_IRQHandler(void)
{
    /* do something */
    GPIO_WriteBit(LED_PORT, LED_PIN, (ledval) ? Bit_SET : Bit_RESET);
    ledval = 1-ledval;
    TIM_ClearITPendingBit(TIM2 ,TIM_IT_Update);
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file , uint32_t line)
{
    /* Infinite loop */
    /* Use GDB to find out why we're here */
    while (1);
}
#endif

