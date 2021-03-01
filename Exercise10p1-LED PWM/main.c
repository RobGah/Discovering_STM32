#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_i2c.h>
#include <stm32f10x_tim.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "uart.h"
#include "spi.h"
#include "LCD7735R.h"
#include "setup_main.h"
#include "xprintf.h"
#include "timers.h"

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

To test:
-Just run the code - backlight on screen should become increasingly brighter for 2s
-After 2s, screen should start to dim for 2s
-Repeats!

For UART Debug, I'm using:

UART    BluePill    BluePill Pin 
TXD     RXD         A9
RXD     TXD         A10
GND     GND         GND
5V      5V          5V

*/

#define USE_FULL_ASSERT

/*SELECT A TEST by commenting / uncommenting these defs*/
#define LCD_BACKLIGHT_FADE_TEST

/****xprintf support****/
void myputchar(unsigned char c)
{
    uart_putc(c, USART1);
}
unsigned char mygetchar ()
{
    return uart_getc(USART1);
}

//PWM variable
int pw = 0; //pulse width 0-999
int ms = 0; //ms count to determine fade in / fade out switchover

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
    
    //init LCD screen
    ST7735_init(); //n.b. this inits the backlight pin too but we reconfig it later
    xprintf("ST7735 initialized!\r\n");
    ST7735_backlight(true);
    ST7735_fillScreen(GREEN);
    Delay(1000); //let the screen actually turn full on as an initial sign of life

    // start LED
    init_onboard_led();
    GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
    bool ledval = false;    
    
    // GPIOA LCD Backlight config for pwm. 
    GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

    //init timer using books definition, 100hz pwm signal with 0.1% pwm resolution.
    timer_init(TIM2,RCC_APB1Periph_TIM2,100000,1000,TIM_CounterMode_Up);

    //MAIN LOOP
    while (1) 
    {
        #ifdef LCD_BACKLIGHT_FADE_TEST
            if((ms < 2000))
            {
                pw++;
            }
            else if (ms>=2000)
            {
                pw--;
            }
            //when we get to 4000ms, reset everything.
            if (ms==4000)
            {
                ms = 0;
                pw = 0; //pw is -1 due to above
            }  
                TIM_SetCompare2(TIM2,pw);
                Delay(2);//delay 2 ms
                ms++;
            #endif
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

