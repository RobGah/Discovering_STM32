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
#include "misc.h"
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
#define LED_PORT  GPIOC
#define LED_PIN  GPIO_Pin_13


int main()
{
      //uart port opened for debugging
    uart_open(USART1,9600);
    uart_puts("UART is Live.",USART1);
    ST7735_init();
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
        //ledval toggle 
        ledval = 1-ledval;
        f_mount(0, &Fatfs);/* Register volume work area */
        uart_puts("Opening an existing file (message.txt)");
        rc = f_open (&Fil , "MESSAGE.TXT", FA_READ);
        if (!rc) {
            xprintf("\nType the file content .\n");
            for (;;) {
            /* Read a chunk of file */
            rc = f_read (&Fil , Buff , sizeof Buff , &br);
            if (rc || !br) break;/* Error or end of file */
            for (i = 0; i < br; i++)/* Type the data */
                myputchar(Buff[i]);
            }
            if (rc) die(rc);
            xprintf("\nClose the file.\n");
            rc = f_close (&Fil);
            if (rc) die(rc);
        }
        xprintf("\nCreate a new file (hello.txt).\n");
        rc = f_open (&Fil , "HELLO.TXT", FA_WRITE | FA_CREATE_ALWAYS);
        if (rc) die(rc);
        xprintf("\nWrite a text data. (Hello world !)\n");
        rc = f_write (&Fil , "Hello world !\r\n", 14, &bw);
        if (rc) die(rc);
        xprintf("%u bytes written .\n", bw);


        //sign of life
        GPIO_WriteBit(GPIOC, GPIO_Pin_13, (ledval) ? Bit_SET : Bit_RESET); //blink
        Delay(1000);
        
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

