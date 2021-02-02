#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_usart.h>
#include <stdint.h>
#include <string.h>
//#include <stdio.h> //ehhhh

#include "uart.h"
#include "spi.h"
#include "LCD7735R.h"

/*

***Last Updated 1/27/21***

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
-Cycle thru alphabet on screen to test letters
-Send corresponding message over UART (e.g. "Successfully wrote %c to the Screen!")

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

    //lets test every letter in the alphabet
    char alphabet[26] = 
    {
        'A','B','C','D','E','F','G','H','I','J','K','L','M',
        'N','O','P','Q','R','S','T','U','V','W','X','Y','Z'
    };

    //setup uart message
    char message[50];
    char messagept2[20];
    char letter[2]; //for alphabet letter
    unsigned int i = 0;
    //MAIN LOOP
    while (1) 
    {
        //ledval
        ledval = 1-ledval;

        //message setup
        strcpy(message, "Sucessfully wrote ");
        strcpy(messagept2, " to the Screen!");
        letter[0] = (char) alphabet[i];
        letter[1] = '\0';
        //create UART message 
        strcat(message, letter);
        strcat(message,messagept2);

        //actually do screen stuff
        //ST7735_writeChar('A', BLACK, WHITE, 10, 10);
         ST7735_writeChar(10, 10, alphabet[i], BLACK, WHITE);
        //write to UART after writing to screen
        uart_puts(message,USART1);
        
        //sign of life
        GPIO_WriteBit(GPIOC, GPIO_Pin_13, (ledval) ? Bit_SET : Bit_RESET); //blink
        Delay(1000);
        i++;
        if(i>=26) //if we run out of chars
        {
            i=0; //reset i to 0
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

