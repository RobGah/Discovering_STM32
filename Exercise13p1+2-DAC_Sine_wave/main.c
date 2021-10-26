#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_exti.h>
#include <stm32f10x_dac.h>
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
#include "dac.h"

/*

Remarks:

SKIPPING EXERCISE AS DAC ISN'T SUPPORTED ON the Blue Pill AND THE VL DISCO BOARD SUCKS

- BLUE PILL DOES NOT HAVE AN ONBOARD DAC :(
- Struggled to try to get a STM32VL disco board up and running, but open-source STLink
    and my computer hate it. It's got a STLINK V1 onboard and despite the docs, I'm 
    skeptical that the openocd st-link utility actually supports V1 at all
- Pretty bummed this can't be realized on the blue pill board and doubly bummed 
    that the VL disco board is cantankerous. 

Setup:

Just the Blue Pill.

Strategy:

Wrote both a function and a program to generate sine wave data

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

// xprintf() support
void myputchar(unsigned char c)
{
    uart_putc(c, USART1);
}
unsigned char mygetchar ()
{
    return uart_getc(USART1);
}

// Sine wave def's
#define NUM_SAMPLES     100
#define MIN_AMP         512
#define MAX_AMP         1536

bool ledval = false;
uint16_t wavetable[NUM_SAMPLES];


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
    GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);

    // INIT DAC
    DAC_init(DAC_Channel_1);

    // Generate waveform samples
    gen_sine_wave(wavetable, NUM_SAMPLES, MIN_AMP, MAX_AMP);
    
    //MAIN LOOP
    while (1) 
    {
        for (uint16_t i = 0; i < NUM_SAMPLES; i++)
        {
            // Give DAC a datapoint every 1ms
            DAC_SetChannel1Data(DAC_Align_12b_R,wavetable[i]);
            xprintf("Passed Value is %d\r\n",wavetable[i]);
            xprintf("Last output value is %d\r\n", DAC_GetDataOutputValue(DAC_Channel_1));
            Delay(1);
        }
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

