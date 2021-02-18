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

***Last Updated 2/16/21***

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

Some notes:
-This was a relatively easy port of software in theory but there are a TON of pitfalls. 
-The book does a GREAT job of guiding you through it but it is a tad outdated.
-Some things not discussed include: 
    corecm3.c's __STREXH and __STREXB need "&r" in their function body (see code)
-I set FF_FS_NORTC to 1 in ff.h after having compiler issues. 
    I get warnings but no errors now. We don't have an RTC module (yet?!) so this setting
    bypasses that problem. 
-in disk_initialize the initialization routine w/ 2 calls to CS_INIT seems to matter? 
    See my code.
-Silly me thing but passing the functions for xprintf into xdev_in and xdev_out 
    should be done in the main routine as the book states. 
-Generally dense code. I recommmend reading:
    http://elm-chan.org/fsw/ff/00index_e.html - GREAT function reference. 
    Also get familiar w/ the error codes the ff.c/h functions spit out.
    I like printing them to the console. 

To test microSD card / FatFS:
-Format Drive and put "message.txt" on drive w/ some phrase in it & read contents. 
-Write "hello.txt" (or whatever) to SD card. 

Uart debug is almost a necessity and xprintf is a godsend. I'm using:

UART    BluePill    BluePill Pin 
TXD     RXD         A9
RXD     TXD         A10
GND     GND         GND
5V      5V          5V

*/

#define USE_FULL_ASSERT

//Variables taken from the elm-chan example for f_open
FATFS FatFs;		/* FatFs work area needed for each volume */
FIL Fil;			/* File object needed for each open file */
UINT bw,br;         /* read/write count */
FRESULT fr;         /* FatFs function common result code*/
char filename[] = "message.txt"; //Name of file already on drive to read
BYTE buffer [4096]; //buffer to hold contents of message.txt 

//xprintf support - taken from book directly
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

    // Configure SysTick Timer
    if(SysTick_Config(SystemCoreClock/1000))
    {
        while(1);
    }

    // start LED
    init_onboard_led();
    GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
    

    /*SELECT A TEST by commenting / uncommenting these defs*/
    #define WRITE_TEST
    #define READ_TEST
    
    //MAIN LOOP
    while (1) 
    {
        xprintf("Mounting drive\r\n");
	    fr = f_mount(&FatFs, "", 1);		/* Give a work area to the default drive */
        xprintf("f_mount completed and returned %d.\r\n",fr);
        
        #ifdef WRITE_TEST
        xprintf("\r\n :::WRITE TEST:::\r\n");
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
        #endif

            #ifdef READ_TEST
            xprintf("\r\n :::READ TEST:::\r\n");
            xprintf("Opening existing file...\r\n");
            fr = f_open(&Fil,filename, FA_READ);
            xprintf("f_open completed and returned %d\r\n",fr);
            if (fr == FR_OK) //we good
            {
                for (;;) 
                {
                    /* Read a chunk of data from the source file */
                    f_read(&Fil, buffer, sizeof(buffer), &br); 
                    if (br == 0) break; /* error or eof */
                }

                xprintf("f_read completed and returned %d\r\n",fr);
                if(fr == FR_OK) 
                {
                    xprintf("File contents are: \r\n \r\n");
                    xprintf(buffer); 
                    xprintf("\r\n\r\n");
                }
                xprintf("Closing file...\r\n");
                fr = f_close(&Fil);
                xprintf("f_close completed and returned %d.\r\n",fr);

                if (fr == FR_OK) 
                { /* Lights onboard LED if data read well */
                    GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET); //ON YAY
                    Delay(2000); //let hold for 2 seconds 
                    GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET); //Off
		        }
                #endif
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

