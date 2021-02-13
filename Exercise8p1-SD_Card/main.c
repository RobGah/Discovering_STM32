#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_usart.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "diskio.h"
#include "uart.h"
#include "spi.h"
#include "LCD7735R.h"
#include "setup_main.h"
#include "ff.h"

/*

***Last Updated 2/11/21***

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

To test microSD card / FatFS:
-Format Drive and put "message.txt" on drive w/ some phrase in it
-Write "hello.txt" to SD card. 

Additionally, grab my uart_puts function from uart.c if you like uart debug outputs. I'm using:

UART    BluePill    BluePill Pin 
TXD     RXD         A9
RXD     TXD         A10
GND     GND         GND
5V      5V          5V

*/

#define USE_FULL_ASSERT

FATFS FatFs;		/* FatFs work area needed for each volume */
FIL Fil;			/* File object needed for each open file */
UINT bw;
FRESULT fr;


int main()
{
      //uart port opened for debugging
    uart_open(USART1,9600);
    uart_puts("UART is Live.",USART1);
    ST7735_init();
    uart_puts("ST7735 Initialized.",USART1);
    ST7735_backlight(1);
    uart_puts("LCD Backlight ON.",USART1);

    // Configure SysTick Timer
    if(SysTick_Config(SystemCoreClock/1000))
    {
        while(1);
    }

    // start LED
    init_onboard_led();

    //MAIN LOOP
    while (1) 
    {
        uart_puts("Opening an existing file (message.txt)", USART1);
	    f_mount(&FatFs, "", 0);		/* Give a work area to the default drive */

	    fr = f_open(&Fil, "newfile.txt", FA_WRITE | FA_CREATE_ALWAYS);	/* Create a file */
	    
        if (fr == FR_OK) 
        {
		    f_write(&Fil, "It works!\r\n", 11, &bw);	/* Write data to the file */
		    fr = f_close(&Fil);							/* Close the file */
		    if (fr == FR_OK && bw == 11) 
            { /* Lights onboard LED if data written well */
                GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET); //ON YAY
                Delay(2000); //let hold for 2 seconds 
                GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET); //Off

		    }
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

