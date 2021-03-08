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
#include "setup_main.h"
#include "xprintf.h"
#include "timers.h"
#include "HC-SR04.h"

/*

Setup:

HC-SR04 rangefinder is connected to the blue pill by way of

Servo   BluePill    BluePill Pin    Note
5V      N/A         N/A             Separate supply
GND     N/A         N/A             GND shared by 5V supply and Bluepill
TRIG    TIM4        PB9             Accepts 10us pulse every 100ms (10Hz)
Echo    TIM1        PA8             Output pulse in response to echo'd US signal 


To test:
- Hook up the US rangefinder and see what we get! Can check w/ a ruler. 

For UART Debug, I'm using:

UART    BluePill    BluePill Pin 
TXD     RXD         A9
RXD     TXD         A10
GND     GND         GND
5V      5V          5V

*/

#define USE_FULL_ASSERT

/*SELECT A TEST by commenting / uncommenting these defs*/
#define RANGEFINDER_TEST

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
    uart_open(USART1,115200);
    xprintf("UART is Live.\r\n");

    // start LED
    init_onboard_led();
    GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
    bool ledval = false;    

    //init timers and pins for US sensor
    #ifdef RANGEFINDER_TEST
        US_sensor_init();
    #endif

int US_measurement = 0;
    //MAIN LOOP
    while (1) 
    {
        #ifdef RANGEFINDER_TEST
        US_measurement  = read_US_sensor_distance_cm();
        xprintf("US sensor distance is %dcm.\r\n", US_measurement);
        Delay(100);
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

