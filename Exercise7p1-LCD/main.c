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

***Last Updated 1/19/21***

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

To test LCD:
-Cycle thru primary colors (R,G,B) w/ delay - WORKS
-Send corresponding message over UART (e.g. "LCD Color is 0x....") - WORKS

Additionally, grab my uart_puts function from uart.c if you like uart debug outputs. I'm using:

UART    BluePill    BluePill Pin 
TXD     RXD         A9
RXD     TXD         A10
GND     GND         GND
5V      5V          5V

*/

#define USE_FULL_ASSERT
#define LED_PORT  GPIOC
#define LED_PIN  GPIO_Pin_13


int main()
{
      //uart port opened for debugging
    uart_open(USART1,9600);
    uart_puts("UART is Live.",USART1);
    ST7735_init(); //stuck here?
    uart_puts("ST7735 Initialized.",USART1);
    ST7735_backlight(1);
    uart_puts("LCD Backlight ON.",USART1);

    // Get onboard LED initialized.
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = LED_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(LED_PORT, &GPIO_InitStructure);
    GPIO_WriteBit(LED_PORT, LED_PIN, Bit_RESET);

    // Configure SysTick Timer
    if(SysTick_Config(SystemCoreClock/1000))
    {
        while(1);
    }
    
    static uint8_t ledval = 0; //LED blinking is just reassuring / sign of life 

    //MAIN LOOP
    while (1) 
    {
        uart_puts("Turning Screen Red...",USART1);
        ledval = 1-ledval;
        ST7735_fillScreen(RED);
        GPIO_WriteBit(GPIOC, GPIO_Pin_13, (ledval) ? Bit_SET : Bit_RESET);
        uart_puts("Success!",USART1);
        Delay(1000);
        
        ledval = 1-ledval;
        GPIO_WriteBit(GPIOC, GPIO_Pin_13, (ledval) ? Bit_SET : Bit_RESET);
        uart_puts("Turning Screen Blue...",USART1);
        ST7735_fillScreen(BLUE);
        uart_puts("Success!",USART1);
        Delay(1000);
        
        ledval = 1-ledval;
        GPIO_WriteBit(GPIOC, GPIO_Pin_13, (ledval) ? Bit_SET : Bit_RESET);
        uart_puts("Turning Screen Green...",USART1);
        ST7735_fillScreen(GREEN);
        uart_puts("Success!",USART1);   
        Delay(1000);
        GPIO_WriteBit(GPIOC, GPIO_Pin_13, (ledval) ? Bit_SET : Bit_RESET);

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

