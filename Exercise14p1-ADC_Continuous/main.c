#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_exti.h>
#include <stm32f10x_dac.h>
#include <stm32f10x_adc.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stm32f10x_tim.h>
#include "interrupts.h"
#include "uart.h"
#include "timers.h"
#include "spi.h"
#include "setup_main.h"
#include "xprintf.h"
#include "adc.h"

/*
Remarks:
-Simple continuous ADC read of the PA6 pin.
-If we're >1.65V, we light an LED. If we're <1.65V, we turn it off. 

Just the Blue Pill.

Strategy:

Wrote both a function and a program to generate sine wave data.

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
#define ADC_MAX_VALUE (2^12)

// xprintf() support
void myputchar(unsigned char c)
{
    uart_putc(c, USART1);
}
unsigned char mygetchar ()
{
    return uart_getc(USART1);
}


int main()
{
    // Configure SysTick Timer
    if(SysTick_Config(SystemCoreClock/1000))
    {
        while(1);
    }

    //setup xprintf - I like these func's better than what the book suggests
    xdev_in(mygetchar); 
    xdev_out(myputchar);

    //uart port opened for debugging
    uart_open(USART1,9600);
    xprintf("UART is Live.\r\n");

    // start LED
    init_onboard_led();

    uint16_t ain = 0;
    adc_init_single(GPIOA,GPIO_Pin_7,ADC1,ADC_Channel_7);
    ADC_SoftwareStartConvCmd(ADC1,ENABLE); // THIS IS THE MISSING PIECE
    //MAIN LOOP
    while (1) 
    {
        // wait for conversion to complete
        // while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
        
        ain = ADC_GetConversionValue(ADC1);
        
        //clear EOC flag
        //ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
        
        xprintf("Current Value is %d\r\n", ain);
        if(ain>=(3800)) // arbitraty - input voltage varies w/ pot used.
        {
            GPIO_WriteBit(LED_PORT,LED_PIN,Bit_RESET);
        }
        else GPIO_WriteBit(LED_PORT,LED_PIN,Bit_SET);
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

