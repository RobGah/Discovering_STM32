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
    //#define FILE_SCAN
    //#define BMP_SCAN
    //define BMP_PARSE
    #define BMP_DISP
    
    //MAIN LOOP
    xprintf("Mounting drive\r\n");
    fr = f_mount(&FatFs, "", 1);		/* Give a work area to the default drive */
    if(fr==RES_OK)
    {
        strcpy(path,"/");
    }
    xprintf("f_mount completed and returned %d.\r\n",fr);
    Delay(500);

    // get the # of BMP files in directory
    fr = f_opendir(&dir, path);
    xprintf("f_opendir() returns %d\r\n",fr);
    bmp_count=scan_bmp_files(path);
    fr = f_closedir(&dir);


    while (1) 
    {
        fr = f_opendir(&dir, path);
        xprintf("f_opendir() returns %d\r\n",fr);
        if(fr == RES_OK)
        {
            #ifdef FILE_SCAN
            xprintf("\r\n :::FILE SCAN:::\r\n");
            xprintf("Checking files on disc...\r\n");
            fr = scan_files(path);
            xprintf("scan_files() completed and returned %d.\r\n",fr);   
            }
            #endif

            #ifdef BMP_SCAN
            xprintf("\r\n :::BMP_SCAN:::\r\n");
            xprintf("Scanning for BMPs\r\n");
            bmp_count = scan_bmp_files(path);
            xprintf("scan_bmp_files() returned %d bmp files.\r\n",bmp_count);    
            #endif

            #ifdef BMP_PARSE
            xprintf("\r\n :::BMP_parse:::\r\n");
            xprintf("Scanning for BMPs\r\n");
            fr = parse_BMP_file(path);
            xprintf("parse_BMP_file() returned %d.\r\n",fr);    
            #endif

            #ifdef BMP_DISP
            xprintf("\r\n :::BMP_DISPLAY:::\r\n");
            while(1)
            {
                for(uint8_t i = 0; i<bmp_count;i++)
                {
                    fr = parse_BMP_file(path);
                    fr = get_BMP_image(path);
                    xprintf("get_BMP_image() returned %d.\r\n",fr);
                    Delay(5000); // 5 seconds per pic    
                }
            f_closedir(&dir);
            Delay(100);
            f_opendir(&dir,&path);
            #endif
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