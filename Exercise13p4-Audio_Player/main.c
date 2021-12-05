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
#include "ff.h"
#include "diskio.h"
#include "audioplayer.h"

/*
Remarks:


- BLUE PILL DOES NOT HAVE AN ONBOARD DAC :(
- UPDATE: Did some thinking and figured out how to get the VL Disco Board programmed
    with the stlink dongle.

Setup:

-STM32VL Disco Board + Optional speaker 
Setup:

ST7735R- Base LCD PCBA is connected to the STM32 "Disco" by way of:

LCD     BluePill    Function
VCC     5V          Power
BKL     PA1         Backlight Control
RESET   PA3         LCD Reset
RS      PA4         Data/Control Toggle
MISO    PB14        SlaveOut
MOSI    PB15        SlaveIn
SCLK    PB13        Clock for SPI2
LCD CS  PA5         LCD Select 
SD_CS   PA6         SD card Select
GND     GND         Ground

Strategy:

Wrote both a function and a program to generate sine wave data.

For UART Debug, I'm using:

UART    Disco       Disco Pin 
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

//debug LED
bool ledval = false;

// Audioplayer flags
bool audioplayerHalf = false;
bool audioplayerWhole = false;
bool audioStopFlag = false;

// Audioplayer variables
uint32_t datasize = 0; // size of data block
uint32_t offset = 0; // offset amount in bytes from start of file
uint32_t readBytesCount = 0; // count of bytes read from file
uint32_t remainingBytesCount; // count of bytes remaining to be read from file
uint8_t wavetable[AUDIOBUFSIZE]; // buffer we load data into
uint8_t Audiobuf[AUDIOBUFSIZE]; // buffer we're playing from

//FatFS variables
FATFS FatFs;
char filename[] = "Doin_it_right.wav";
// Bad form but I just kind of threw this everywhere
// Well aware it gets #include'd - can remove after developing code.
#define AUDIOPLAYER

#ifdef AUDIOPLAYER

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

    //mount drive
    f_mount(&FatFs,"",1);

    // init audioplayer
    audioplayerInit();

    // load file - get datasize and assign value to remainingBytesCount
    audioplayerLoadFile(&filename, &wavetable, datasize, offset);
    remainingBytesCount = datasize;

    // initial full load of audio buffer data
    for(int i = 0; i<AUDIOBUFSIZE; i++)
    {
        Audiobuf[i] = wavetable[i];
    }

    // increment readBytesCount, decrement remainingBytesCount
    readBytesCount += AUDIOBUFSIZE;
    remainingBytesCount -= AUDIOBUFSIZE;

    // MAIN LOOP
    while (1) 
    {
        if (audioStopFlag == true)
        {
           audioplayerStop();
           return 1; // program stops for good.  
        }
        // Poll the half and whole transfer flags
        if (audioplayerWhole == true)
        {
            if((remainingBytesCount) < AUDIOBUFSIZE/2)
            {
                memset(&Audiobuf[AUDIOBUFSIZE/2],0,AUDIOBUFSIZE/2);
                audioplayerNextBlock(&filename,&wavetable,offset,remainingBytesCount);
                audioStopFlag = true;
                readBytesCount += remainingBytesCount;
                remainingBytesCount -= remainingBytesCount;
            }
            else
            {
                // stock the back-half of the wavetable buffer
                audioplayerNextBlock(&filename,&wavetable[AUDIOBUFSIZE/2],offset,AUDIOBUFSIZE/2);
                for (int i = AUDIOBUFSIZE /2; i < AUDIOBUFSIZE; i++)
                {
                    Audiobuf[i] = wavetable[i];
                    audioplayerWhole = false;
                }
                readBytesCount += AUDIOBUFSIZE/2;
                remainingBytesCount -= AUDIOBUFSIZE/2;
            }
        }

        else if (audioplayerHalf == true)
        {
            if((remainingBytesCount) < AUDIOBUFSIZE/2)
            {
                memset(&Audiobuf,0,AUDIOBUFSIZE/2);
                audioplayerNextBlock(&filename,&wavetable,offset,remainingBytesCount);
                audioStopFlag = true;
                readBytesCount += remainingBytesCount;
                remainingBytesCount -= remainingBytesCount;
            }
            else
            {
                // stock the front-half of the wavetable buffer
                audioplayerNextBlock(&filename,&wavetable,offset,AUDIOBUFSIZE/2);
                for (int i = 0; i < AUDIOBUFSIZE/2; i++)
                {
                    Audiobuf[i] = wavetable[i];
                    audioplayerHalf = false;
                }   
                readBytesCount += AUDIOBUFSIZE/2;
                remainingBytesCount -= AUDIOBUFSIZE/2;
            }
        }
    }
   return(0);
} 
#endif


#ifdef SINGLETONETEST
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

    // Generate waveform samples for wavetable and initial Audiobuf 
    gen_sine_wave(&wavetable, AUDIOBUFSIZE, MIN_AMP, MAX_AMP);
    gen_sine_wave(&Audiobuf,AUDIOBUFSIZE,MIN_AMP, MAX_AMP);

    // Timer Init
    // This is 'close enough' - did both math (see worksheet)
    // and verified with mikro calculator program for timers
    // DANGER - prescalar/divided clock must be an integer value
    timer_init(TIM3, RCC_APB1Periph_TIM3, 4000000, 
        91, TIM_CounterMode_Up); 

    // Config output to trigger on an update
    TIM_SelectOutputTrigger(TIM3 , TIM_TRGOSource_Update);
    TIM_ITConfig(TIM3 , TIM_IT_Update , ENABLE);
    TIM_Cmd(TIM3 , ENABLE); // why not

    // Enable Interrupt
    config_NVIC(DMA1_Channel3_IRQn,0);

    // INIT DAC
    DAC_init_w_Trig(DAC_Channel_1,DAC_Trigger_T3_TRGO);
    init_dac_dma(Audiobuf,AUDIOBUFSIZE);
    
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
#endif

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

