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
#include "xprintf.h"

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

//xprintf support
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
    //setup xprintf
    xdev_in(mygetchar); 
    xdev_out(myputchar);

      //uart port opened for debugging
    uart_open(USART1,9600);
    xprintf("UART is Live.\r\n");
    ST7735_init();
    xprintf("ST7735 Initialized.\r\n");
    ST7735_backlight(1);
    xprintf("LCD Backlight ON.\r\n");

    // Configure SysTick Timer
    if(SysTick_Config(SystemCoreClock/1000))
    {
        while(1);
    }

    // start LED
    init_onboard_led();
    GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
    //MAIN LOOP
    while (1) 
    {
        xprintf("Mounting drive\r\n");
	    fr = f_mount(&FatFs, "", 1);		/* Give a work area to the default drive */
        xprintf("f_mount completed and returned %d.\r\n",fr);
        xprintf("Creating newfile...\r\n");
	    fr = f_open(&Fil, "newfile.txt", FA_WRITE | FA_CREATE_ALWAYS);	/* Create a file */
        xprintf("f_open completed and returned %d\r\n",fr);
        if (fr == FR_OK) 
        {
            xprintf("Writing in file...\r\n");
		    fr=f_write(&Fil, "It works!\r\n", 11, &bw);	/* Write data to the file */
		    xprintf("f_write completed and returned %d\r\n",fr);
            xprintf("Closing file...\r\n");
            fr = f_close(&Fil);							/* Close the file */
            xprintf("f_close completed and returned %d.\r\n",fr);

		    if (fr == FR_OK && bw == 11) 
            { /* Lights onboard LED if data written well */
                GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET); //ON YAY
                Delay(2000); //let hold for 2 seconds 
                GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET); //Off

		    }
	    }
        for(;;);
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

