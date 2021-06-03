#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_i2c.h>
#include <stm32f10x_tim.h>
#include <stm32f10x_exti.h>
#include <misc.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "uart.h"
#include "setup_main.h"
#include "interrupts.h"
#include "xprintf.h"


/*

Setup:


BluePill    BluePill Pin    Note
LED         PC13             Onboard LED 
Switch*     PA0              External pushbutton w/ 10k pulldown to gnd 
*RC circuit as shown by author 

To test:
- Set up flow control. Echo characters that are input over terminal.

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

/*SELECT A TEST by commenting / uncommenting these defs*/
#define BUTTON_TEST
//#define USART_TEST

#ifdef USART_TEST
    #include "uartfc.h"
    uint8_t buf[8];
#endif

#ifdef BUTTON_TEST
/****xprintf support****/
void myputchar(unsigned char c)
{
    uart_putc(c, USART1);
}
unsigned char mygetchar ()
{
    return uart_getc(USART1);
}

bool ledval = false;
#endif

int main()
{
    // Configure SysTick Timer
    if(SysTick_Config(SystemCoreClock/1000))
    {
        while(1);
    }
    #ifdef USART_TEST
        //uart port opened for debugging
        uart_open(1, 115200,0); 
    #endif
    
    #ifdef BUTTON_TEST

    //setup xprintf 
    xdev_in(mygetchar); 
    xdev_out(myputchar);

    //open uart
    uart_open(USART1,9600);
    xprintf("UART is live!\r\n");

    //Set-up LED
    init_onboard_led();

    //Config PA0
    init_GPIO_pin(GPIOA,GPIO_Pin_0,GPIO_Mode_IN_FLOATING,GPIO_Speed_50MHz);

    //config NVIC
    config_NVIC(EXTI0_IRQn,3);

    //config external interrupt
    config_EXTIO(GPIO_PortSourceGPIOA,GPIO_PinSource0,EXTI_Line0,
        EXTI_Mode_Interrupt,EXTI_Trigger_Rising);

    #endif
    
    //MAIN LOOP
    while (1) 
    { 
        #ifdef USART_TEST
        //read incoming
        int x = uart_read(1,buf,sizeof(buf));
        //echo if there is something entered into the terminal. 
        if(x!=0)
        {
        uart_write(1,buf,sizeof(buf));
        }
        #endif

        #ifdef BUTTON_TEST
        //nothing!
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

#ifdef BUTTON_TEST
    void EXTI0_IRQHandler(void)
    {
        xprintf("External Interrupt Tripped!\r\n");
        //if we're tripped
        if(EXTI_GetITStatus(EXTI_Line0) != RESET)
        {
            //toggle LED
            GPIO_WriteBit(LED_PORT, LED_PIN, (ledval) ? Bit_SET : Bit_RESET);
            ledval = 1-ledval;            
            //clear the interrupt
            EXTI_ClearITPendingBit(EXTI_Line0);
        }

    }
#endif

