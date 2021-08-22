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
#include "sdcard.h"


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

// SD Card Variables
FATFS FatFs;		/* FatFs work area needed for each volume */
FIL Fil;			/* File object needed for each open file */
UINT bmp_count = 0; /* ad oculos*/
FRESULT fr;         /* FatFs function common result code*/
char path[256];   /* Root Directory path */

// xprintf() support
void myputchar(unsigned char c)
{
    uart_putc(c, USART1);
}
unsigned char mygetchar ()
{
    return uart_getc(USART1);
}

bool ledval = false;

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
    
    //might not be needed but I kept it.
    ST7735_init();
    xprintf("ST7735 Initialized.\r\n");
    ST7735_backlight(1);
    xprintf("LCD Backlight ON.\r\n");

    // start LED
    init_onboard_led();
    GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
    

    /*SELECT A TEST by commenting / uncommenting these defs*/
    #define FILE_SCAN
    #define BMP_SCAN
    
    //MAIN LOOP
    while (1) 
    {
        xprintf("Mounting drive\r\n");
	    fr = f_mount(&FatFs, "", 1);		/* Give a work area to the default drive */
        if(fr==RES_OK)
        {
            strcpy(path,"/");
        }
        xprintf("f_mount completed and returned %d.\r\n",fr);
        Delay(500);
        
        #ifdef FILE_SCAN
        xprintf("\r\n :::FILE SCAN:::\r\n");
        xprintf("Checking files on disc...\r\n");
        fr = scan_files(path);
        xprintf("scan_files() completed and returned %d.\r\n",fr);

		    if (fr == FR_OK) 
            { /* Lights onboard LED if file scan went well */
                GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET); //ON YAY
                Delay(2000); //let hold for 2 seconds 
                GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET); //Off
                memset(path,0,sizeof(path));
                strcpy(path,"/");

		    }
        Delay(500);
        #endif

        #ifdef BMP_SCAN
        xprintf("\r\n :::BMP_SCAN:::\r\n");
        xprintf("Scanning for BMPs\r\n");
        bmp_count = scan_bmp_files(path);
        xprintf("scan_bmp_files() returned %d bmp files.\r\n",bmp_count);    

        if (bmp_count>0) 
        { /* Lights onboard LED if data read well */
            GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET); //ON YAY
            Delay(2000); //let hold for 2 seconds 
            GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET); //Off
        }
        #endif
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