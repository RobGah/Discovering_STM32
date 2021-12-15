#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_exti.h>
#include <stm32f10x_dac.h>
#include <stm32f10x_adc.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stm32f10x_tim.h>
#include "interrupts.h"
#include "uart.h"
#include "timer.h"
#include "spi.h"
#include "setup_main.h"
#include "xprintf.h"
#include "adc.h"
#include "misc.h"

/****FREERTOS INCLUDES****/
//#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "list.h"
#include "timers.h"


/*
Remarks:
-Simple continuous ADC read of the PA6 pin.
-If we're >1.65V, we light an LED. If we're <1.65V, we turn it off. 

Just the Blue Pill.

Strategy:

Wrote both a function and a program to generate sine wave data.

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

// xprintf() support
void myputchar(unsigned char c)
{
    uart_putc(c, USART1);
}
unsigned char mygetchar ()
{
    return uart_getc(USART1);
}


static void Thread1(void *arg) 
{
    int dir = 0;
    while (1) 
    {
        vTaskDelay (300/ portTICK_RATE_MS);
        GPIO_WriteBit(GPIOC , GPIO_Pin_9 , dir ? Bit_SET : Bit_RESET);
        dir = 1 - dir;
    }
}
static void Thread2(void *arg) 
{
    int dir = 0;
    while (1) 
    {
        vTaskDelay (500/ portTICK_RATE_MS);
        GPIO_WriteBit(GPIOC , GPIO_Pin_8 , dir ? Bit_SET : Bit_RESET);
        dir = 1 - dir;
    }
}
int main(void)
{
    // set up interrupt priorities for FreeRTOS !!
    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );
    
    /****initialize hardware****/
    // Configure SysTick Timer
    if(SysTick_Config(SystemCoreClock/1000))
    {
        while(1);
    }
    init_GPIO_pin(GPIOC,GPIO_Pin_8,GPIO_Mode_Out_PP,GPIO_Speed_50MHz);
    init_GPIO_pin(GPIOC,GPIO_Pin_9,GPIO_Mode_Out_PP,GPIO_Speed_50MHz);

    // Create tasks
    xTaskCreate(Thread1 , // Function to execute
                "Thread 1", // Name
                128, // Stack size
                NULL , // Parameter (none)
                tskIDLE_PRIORITY + 1 , // Scheduling priority
                NULL // Storage for handle (none)
                );
    
    xTaskCreate(Thread2 , 
                "Thread 2", 
                128,
                NULL , 
                tskIDLE_PRIORITY + 1 , 
                NULL
                );
    // Start scheduler
    vTaskStartScheduler();
    // Schedule never ends
}


#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file , uint32_t line)
{
    /* Infinite loop */
    /* Use GDB to find out why we're here */
    while (1);
}
#endif
