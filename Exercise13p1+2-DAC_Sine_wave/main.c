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
#include "dac.h""

/*

See: http://elm-chan.org/fsw/ff/00index_e.html

Remarks:
-12p1 is easy. 12p2 is a pretty brutal and involved exercise. Very challenging. 
-I lumped the 2 exercises together because it just makes sense. 

Setup:

ST7735R- Base LCD PCBA is connected to the STM32 "Blue Pill" by way of:

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
-Bonus points if we can HW flow control working w/ xprintf!
-Get spidma.c working (hopefully)
-Get elm-chan's FatFs to parse the SD card for bmp files. 
    Ensure that we can identify them on disc. 
    Can do this with spi.c first just to prove it out.
-Test the FatFS SD card bmp file identifier w/ SPIDMA.c just to prove it works
-Next, parse each file for its file info
    Look into using parseBMP() from author
-Finally, feed the BMP pic bytes to the LCD screen.
    Will need to convert 24 bit BMP to 16 bit BMP. See author's remarks.
    Should reject images that are not 128x160 in size and 24 bit color. Test this.
    Loop thru the pictures on the card. Forever. 
-Write a timer routine to measure the time to display an image for different DMA block sizes 


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
    gen_sine_wave(&wavetable, NUM_SAMPLES, MIN_AMP, MAX_AMP);
    
    //MAIN LOOP
    while (1) 
    {
        for (uint16_t i = 0; i < NUM_SAMPLES; i++)
        {
            // Give DAC a datapoint every 1ms
            DAC_SetChannel1Data(DAC_Align_12b_L,wavetable[i]);
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

