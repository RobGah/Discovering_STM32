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
#include "nunchukservo.h"

/*

Setup:

Servo is connected to the blue pill by way of

Servo   BluePill    BluePill Pin    Note
5V      N/A         N/A             Separate supply
GND     N/A         N/A             GND shared by 5V supply and Bluepill
CTRL    TIM2        A1              For sign of life test(at least) - see the code.


Wii Nunchuk is connected to STM32 by way of:
Line    Nunchuk     STM32
3.3V    +           3.3V
GND     -           GND
Clock   c           PB6
Data    d           PB7

This is effectively I2C1. 
I2C 2 is also possible and could PB6 -> PB10 and PB7->PB11 - see code. 


To test:
- Get the Servo motor working w/ the sign of life test - ensure we can do 0-45-90deg with code
- Map nunchuk joystick inputs to servo pulse width commands - one servo is "up-down", other is "left-right"

For UART Debug, I'm using:

UART    BluePill    BluePill Pin 
TXD     RXD         A9
RXD     TXD         A10
GND     GND         GND
5V      5V          5V

*/

#define USE_FULL_ASSERT

/*SELECT A TEST by commenting / uncommenting these defs*/
//#define SERVO_SIGN_OF_LIFE_TEST
#define SERVO_NUNCHUK_TEST

/****xprintf support****/
void myputchar(unsigned char c)
{
    uart_putc(c, USART1);
}
unsigned char mygetchar ()
{
    return uart_getc(USART1);
}

#ifdef SERVO_SIGN_OF_LIFE_TEST
//PWM variables
    int pw_0deg = 100; //10% duty cycle 100Hz
    int pw_45deg = 150; //15% duty cycle 100Hz
    int pw_90deg = 200; //20% duty cycle 100Hz
    
    //135deg or 25% duty ~sorta~ works on my servo but it seems to get strained . 
#endif

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

    // start LED
    init_onboard_led();
    GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
    bool ledval = false;    
    
    #ifdef SERVO_SIGN_OF_LIFE_TEST
        // GPIOA config for pwm / servo. 
        GPIO_InitTypeDef GPIO_InitStructure;
	    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	    GPIO_Init(GPIOA, &GPIO_InitStructure);

        //init timer using books definition
        //divide system's core clock (72MHz) by 100k to get 100KHz tick,
        //period is 1000 ticks, so 100Hz signal is generated. 
        pwm_init(TIM2,RCC_APB1Periph_TIM2,100000,1000,TIM_CounterMode_Up);
    #endif

    #ifdef SERVO_NUNCHUK_TEST
        init_nunchuk_servo_peripherals(I2C2,100000,0xA4,GPIOB,GPIO_Pin_8,GPIOB,GPIO_Pin_9);
    #endif


    //MAIN LOOP
    while (1) 
    {
        #ifdef SERVO_SIGN_OF_LIFE_TEST
            TIM_SetCompare2(TIM2,pw_0deg); //10% duty cycle @ 100Hz = 1ms
            Delay(1000);
            TIM_SetCompare2(TIM2,pw_45deg);//15% duty cycle @ 100Hz = 1.5ms
            Delay(1000);
            TIM_SetCompare2(TIM2,pw_90deg);//20% duty cycle @ 100Hz = 2ms
            Delay(1000);
        #endif

        #ifdef SERVO_NUNCHUK_TEST
            write_joystick_to_servo();
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

