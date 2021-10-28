#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_exti.h>
#include <stm32f10x_dac.h>
#include <stm32f10x_dma.h>
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
#include "audioplayer.h"

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
#define MIN_AMP         512
#define MAX_AMP         1536

bool ledval = false;
bool audioplayerHalf = false;
bool audioplayerWhole = false;
uint8_t wavetable[AUDIOBUFSIZE];
uint8_t Audiobuf[AUDIOBUFSIZE];
int wavept_cnt = 0;


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

    // Timer Init
    // This is 'close enough' - did both math (see worksheet)
    // and verified with mikro calculator program for timers
    timer_init(TIM3, RCC_APB1Periph_TIM3, 18000000, 
        136, TIM_CounterMode_Up);

    // Config output to trigger on an update
    TIM_SelectOutputTrigger(TIM3 , TIM_TRGOSource_Update);
    TIM_ITConfig(TIM3 , TIM_IT_Update , ENABLE);
    TIM_Cmd(TIM3 , ENABLE); // why not

    // Enable Interrupt
    config_NVIC(DMA1_Channel3_IRQn,0);

    // INIT DAC
    DAC_init_w_Trig(DAC_Channel_1,DAC_Trigger_T3_TRGO);
    init_dac_dma(Audiobuf,AUDIOBUFSIZE);
    // Generate waveform samples for wavetable and initial Audiobuf 
    gen_sine_wave(&wavetable, AUDIOBUFSIZE, MIN_AMP, MAX_AMP);
    gen_sine_wave(&Audiobuf,AUDIOBUFSIZE,MIN_AMP, MAX_AMP);
    
    // MAIN LOOP
    while (1) 
    {
        // Poll the half and whole transfer flags
        if (audioplayerWhole == true)
        {
            for (int i = AUDIOBUFSIZE /2; i < AUDIOBUFSIZE; i++)
            {
               Audiobuf[i] = wavetable[i];
               audioplayerWhole = false;
            }
        }

        else if (audioplayerHalf == true)
        {
            for (int i = 0; i < AUDIOBUFSIZE/2; i++)
            {
                Audiobuf[i] = wavetable[i];
                audioplayerHalf = false;
            }   
        }
    }
   return(0);
}

void DMA1_Channel3_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_IT_TC3))
    {   
        audioplayerWhole = true;
        DMA_ClearITPendingBit(DMA1_IT_TC3);
    }
    else if (DMA_GetITStatus(DMA1_IT_HT3))
    {   
        audioplayerHalf = true;
        DMA_ClearITPendingBit(DMA1_IT_HT3);
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file , uint32_t line)
{
    /* Infinite loop */
    /* Use GDB to find out why we're here */
    while (1);
}
#endif

