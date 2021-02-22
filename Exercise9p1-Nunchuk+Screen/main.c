#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_i2c.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "uart.h"
#include "spi.h"
#include "LCD7735R.h"
#include "setup_main.h"
#include "xprintf.h"
#include "i2c.h"
#include "nunchuk.h"
/*

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

Wii Nunchuk is connected to STM32 by way of:
Line    Nunchuk     STM32
3.3V    +           3.3V
GND     -           GND
Clock   c           PB6
Data    d           PB7

This is effectively I2C1. 
I2C 2 is also possible and could PB6 -> PB10 and PB7->PB11


Some notes:
-Embarassing, but I finally realized that the Library subfolder has many of these
    modules that the author lists in the book pre-defined and ready for fill-in-the-blank coding.
    I just took the I2C module verbatim from that subfolder.

To test microSD card / FatFS:
-First, read out the accel/joy/button nunchuck output onto UART / TeraTerm
-Then go about mapping those values to the screen.

For UART Debug, I'm using:

UART    BluePill    BluePill Pin 
TXD     RXD         A9
RXD     TXD         A10
GND     GND         GND
5V      5V          5V

*/

#define USE_FULL_ASSERT

/****Variables for I2C init****/
#define NUNCHUK_ADDRESS 0x52 //book says 0xA4
/****xprintf support****/
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

    //setup xprintf 
    xdev_in(mygetchar); 
    xdev_out(myputchar);

    //uart port opened for debugging
    uart_open(USART1,9600);
    xprintf("UART is Live.\r\n");
    
    //Initialize ST7735 Screen.
    ST7735_init();
    xprintf("ST7735 Initialized.\r\n");
    ST7735_backlight(1);
    xprintf("LCD Backlight ON.\r\n");

   //Initialize 'chuk
   nunchuk_init(I2C1,10000,NUNCHUK_ADDRESS);


    // start LED
    init_onboard_led();
    GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
    bool ledval = false;

    /*SELECT A TEST by commenting / uncommenting these defs*/
    
    //MAIN LOOP
    while (1) 
    {
        GPIO_WriteBit(GPIOC, GPIO_Pin_13, ledval? Bit_SET: Bit_RESET);
        //basic sign of life test for nunchuk reads
        report_nunchuk_data(I2C1,NUNCHUK_ADDRESS);
        ledval= 1-ledval;
        Delay(500);
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

