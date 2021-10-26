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


- BLUE PILL DOES NOT HAVE AN ONBOARD DAC :(
- UPDATE: Did some thinking and figured out how to get the VL Disco Board programmed
    with the stlink dongle.

Setup:

-STM32VL Disco Board + Optional speaker 

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
uint8_t wavept_cnt =  0;

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
    uart_open(USART1,115200);
    xprintf("UART is Live.\r\n");

    // start LED
    init_onboard_led();
    GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);

    // Timer Init
    // This is 'close enough' - did both math (see worksheet)
    // and verified with mikro calculator program for timers
    timer_init(TIM3, RCC_APB1Periph_TIM3, 4000000, 
        91, TIM_CounterMode_Up);

    // Config output to trigger on an update
    TIM_SelectOutputTrigger(TIM3 , TIM_TRGOSource_Update);
    TIM_ITConfig(TIM3 , TIM_IT_Update , ENABLE);
    TIM_Cmd(TIM3 , ENABLE); // why not

    // Enable Interrupt
    config_NVIC(TIM3_IRQn,0);

    // INIT DAC
    DAC_init_w_Trig(DAC_Channel_1, DAC_Trigger_T3_TRGO);

    // Generate waveform samples
    gen_sine_wave(&wavetable, NUM_SAMPLES, MIN_AMP, MAX_AMP);
    
    //MAIN LOOP
    while (1) 
    {
       //nothing!
    }
   return(0);
}

void TIM3_IRQHandler(void)
{
    //DEBUG - MUST slow timer down to use (e.g. 136 -> 13600+ for period)
    //xprintf("TIM3 Interrupt!\r\n");
    //xprintf("wavept cnt is %d\r\n",wavept_cnt);
    if(wavept_cnt>=NUM_SAMPLES)
    {
        wavept_cnt=0; // reset and clear
    }

    DAC_SetChannel1Data(DAC_Align_12b_R,wavetable[wavept_cnt]);
    //xprintf("Wavept value is %d\r\n",wavetable[wavept_cnt]);
    wavept_cnt++;        
    TIM_ClearITPendingBit(TIM3 ,TIM_IT_Update);

}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file , uint32_t line)
{
    /* Infinite loop */
    /* Use GDB to find out why we're here */
    while (1);
}
#endif

