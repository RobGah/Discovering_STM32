#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_exti.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stm32f10x_tim.h>
#include "interrupts.h"
#include "diskio.h"
#include "uart.h"
#include "timers.h"
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

#define BMP_BUFFER_SIZE 128

char path[256];                           // Root directory path

uint16_t bytestream[BMP_BUFFER_SIZE];    //default buffer to hold 16-bit BMP data

#define TIME_TEST

#ifdef TIME_TEST
uint16_t bytestream8[8];     // 8 buffer to hold 16-bit BMP data 
uint16_t bytestream16[16];    // 16 buffer to hold 16-bit BMP data
uint16_t bytestream32[32];    // 32 buffer to hold 16-bit BMP data
uint16_t bytestream64[64];    // 64 buffer to hold 16-bit BMP data
uint16_t bytestream256[256];  // 256 buffer to hold 16-bit BMP data
#endif


uint32_t ms_count = 0;
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

    config_NVIC(SysTick_IRQn,3);

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
    //#define BMP_DISP
    //NOTE: TIME_TEST is defined at top of file
    
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
                    fr = parse_BMP_file(&path);
                    fr = get_BMP_image_DMA(&path,&bytestream);
                    xprintf("get_BMP_image() returned %d.\r\n",fr);
                    Delay(5000); // 5 seconds per pic    
                }
            f_closedir(&dir);
            Delay(100);
            f_opendir(&dir,&path);
            }
            #endif


            #ifdef TIME_TEST
            while(1)
            {
                xprintf("\r\n :::TIME TEST:::\r\n");
                xprintf("8 byte:\r\n");
                fr = parse_BMP_file(&path);
                ms_count = 0;
                fr = get_BMP_image_DMA(&path,&bytestream8);
                xprintf("Image w/ %d buffer size DMA took %d ms to load\r\n",sizeof(bytestream8), ms_count);
                xprintf("get_BMP_image() returned %d.\r\n",fr);
                
                //Reset Everything
                f_closedir(&dir);
                Delay(1000);
                f_opendir(&dir,&path);

                xprintf("16 byte:\r\n");
                fr = parse_BMP_file(&path);
                ms_count = 0;
                fr = get_BMP_image_DMA(&path,&bytestream16);
                xprintf("Image w/ %d buffer size DMA took %d ms to load\r\n",sizeof(bytestream16), ms_count);
                xprintf("get_BMP_image() returned %d.\r\n",fr);
                
                //Reset Everything
                f_closedir(&dir);
                Delay(1000);
                f_opendir(&dir,&path);

                xprintf("32 byte:\r\n");
                fr = parse_BMP_file(&path);
                ms_count = 0;
                fr = get_BMP_image_DMA(&path,&bytestream32);
                xprintf("Image w/ %d buffer size DMA took %d ms to \r\n",sizeof(bytestream32), ms_count);
                xprintf("get_BMP_image() returned %d.\r\n",fr);
                
                //Reset Everything
                f_closedir(&dir);
                Delay(1000);
                f_opendir(&dir,&path);

                xprintf("64 byte:\r\n");
                fr = parse_BMP_file(&path);
                ms_count = 0;
                fr = get_BMP_image_DMA(&path,&bytestream64);
                xprintf("Image w/ %d buffer size DMA took %d ms to load\r\n",sizeof(bytestream64), ms_count);
                xprintf("get_BMP_image() returned %d.\r\n",fr);
                
                //Reset Everything
                f_closedir(&dir);
                Delay(1000);
                f_opendir(&dir,&path);

                xprintf("128 byte:\r\n");
                fr = parse_BMP_file(&path);
                ms_count = 0;
                fr = get_BMP_image_DMA(&path,&bytestream);
                xprintf("Image w/ %d buffer size DMA took %d ms to load\r\n",sizeof(bytestream), ms_count);
                xprintf("get_BMP_image() returned %d.\r\n",fr);

                //Reset Everything
                f_closedir(&dir);
                Delay(1000);
                f_opendir(&dir,&path);

                xprintf("256 byte:\r\n");
                fr = parse_BMP_file(&path);
                ms_count = 0;
                fr = get_BMP_image_DMA(&path,&bytestream256);
                xprintf("Image w/ %d buffer size DMA took %d ms to load\r\n",sizeof(bytestream256), ms_count);
                xprintf("get_BMP_image() returned %d.\r\n",fr);
                
                //Reset Everything
                f_closedir(&dir);
                Delay(1000);
                f_opendir(&dir,&path);

                fr = parse_BMP_file(&path);
                ms_count = 0;
                fr = get_BMP_image(&path);
                xprintf("Image w/o DMA took %d ms to load\r\n", ms_count);
                xprintf("get_BMP_image() returned %d.\r\n",fr);
                
                //Reset Everything
                f_closedir(&dir);
                Delay(1000);
                f_opendir(&dir,&path);
            }
            #endif
        for(;;);
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
        ms_count++; //increment ms_count on every 1ms SysTick
    }
}

