#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_i2c.h>
#include <stm32f10x_tim.h>
#include <misc.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "uart.h"
#include "setup_main.h"

/*

Setup:

No connections. Just blinking an LED

BluePill    BluePill Pin    Note
LED         PC13             Onboard LED 


To test:
- Set up flow control. Echo characters that are input over terminal.

For UART Debug, I'm using:

UART    BluePill    BluePill Pin 
TXD     RXD         A9
RXD     TXD         A10
CTS     CTS         A11
RTS     RTS         A12   
GND     GND         GND
5V      5V          5V

*/

#define USE_FULL_ASSERT

/*SELECT A TEST by commenting / uncommenting these defs*/

uint8_t buf[8];

int main()
{
    // Configure SysTick Timer
    if(SysTick_Config(SystemCoreClock/1000))
    {
        while(1);
    }
    
    //uart port opened for debugging
    uart_open(1, 115200,0); 
    

    //MAIN LOOP
    while (1) 
    { 
        //read incoming
        uart_read(1,buf,sizeof(buf));
        //echo
        uart_write(1,buf,sizeof(buf));
    }

    return(0);
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file , uint32_t line)
{
    /* Infinite loop */
    /* Use GDB to find out why we're here */
    while (1);
}
#endif

