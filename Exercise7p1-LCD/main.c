#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_usart.h>
#include <stdint.h>
#include <string.h>

#include "uart.h"
#include "spi.h"
#include "LCD7735R.h"

/*
Setup:

ST7735R- Base LCD PCBA is connected to the STM32 "Blue Pill" by way of:

LCD     BluePill    Function
VCC     5V          Power
BKL     PA1         Backlight Control
RESET   PC1         LCD Reset
RS      PC2         Data/Control Toggle
MISO    PB14        SlaveOut
MOSI    PB15        SlaveIn
SCLK    PB13        Clock for SPI2
LCD CS  PC0         LCD Select 
SD_CS   PC6         SD card Select
GND     GND         Ground

To test LCD:
-Cycle thru primary colors (R,G,B) w/ delay
-Send corresponding message over UART ("LCD Color is 0x....")

UART    BluePill    BluePill Pin 
TXD     RXD         A9
RXD     TXD         A10
GND     GND         GND
5V      5V          5V

*/

#define USE_FULL_ASSERT

void Delay(uint32_t nTime);

int main()
{
    ST7735_backlight(1);
    ST7735_init(); //initializes SPI for us
        
    // Get onboard LED initialized.
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // Configure SysTick Timer
    if(SysTick_Config(SystemCoreClock/1000))
    {
        while(1);
    }

    //MAIN LOOP
    while (1) 
    {
        //toggle LED for sign of life while flipping through screen colors
        static int ledval = 0;
        ST7735_fillScreen(RED);
        Delay(1000);
        ledval = 1-ledval;
        ST7735_fillScreen(BLUE);
        Delay(1000);
        ledval = 1-ledval;
        ST7735_fillScreen(GREEN);
        Delay(1000);
        ledval = 1-ledval;
    }

   return(0);
}

// Timer code
static __IO uint32_t TimingDelay;

void Delay(uint32_t nTime)
{
    TimingDelay = nTime;
    while(TimingDelay !=0);
}

void SysTick_Handler(void)
{
    if (TimingDelay != 0x00)
    {
        TimingDelay--;
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

