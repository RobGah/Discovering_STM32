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
#include "uartfc-rtos.h"
#include "timer.h"
#include "spi.h"
#include "setup_main.h"
#include "xprintf.h"
#include "adc.h"
#include "misc.h"

/****FREERTOS INCLUDES****/
#include "FreeRTOSConfig.h"
#include "FreeRTOS_Source/include/FreeRTOS.h"
#include "FreeRTOS_Source/include/FreeRTOS.h"
#include "FreeRTOS_Source/include/task.h"
#include "FreeRTOS_Source/include/queue.h"
#include "FreeRTOS_Source/include/list.h"
#include "FREERTOS_Source/include/timers.h"
#include "FreeRTOS_Source/include/semphr.h"


/*
Remarks:
- Hardware flow control was realized somewhat strangely by myself back in Exercise 11.3. 
As a result, I've gone ahead and cleaned up the code while also implementing FREERTOS Queues.
- Exercise 11.3 DOES work but looking back on it I believe now that I should have
wrapped putchar() and getchar() with functions to handle the "read/write entire lines" 
requirement instead of making uart_read()/uart_write() functions - these aren't really
generalizable and are kind of why I never got around to xprintf w/ flow control. 

Setup:
Just the Blue Pill / Disco Board.

Strategy:

- Re-tool uartfc.[ch] - create a new module uartfc-rtos.c that incorporates FreeRTOS Queues.
Its simple enough to just rip out the queue struct and Enqueue()/Dequeue() and replace it
with FreeRTOS Queues.
- Write driving threads for the flow controlled USART - perhaps one thread handles 
composing messages to send over UART (like a status update on LED states), 
one thread handles receiving messages and controlling LEDs
in response to different messages coming in (i.e. "LED1 on/off", "LED2 on/off")

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
    putchar_rtos(c);
}
unsigned char mygetchar()
{
    return getchar_rtos();
}

// Mutexes and Semaphores
static SemaphoreHandle_t serialMutex = NULL;

// Queues
static QueueHandle_t commandQueue;
static QueueHandle_t printingQueue;


// Threads
static void Thread1(void *arg) 
{
    /* Thread 1 checks incoming serial comms */
    while (1) 
    {        
        vTaskDelay(1000/portTICK_RATE_MS);
    }
}

static void Thread2(void *arg) 
{
    /* Thread 2 actuates the LED*/
    while (1) 
    {
        vTaskDelay(1000/portTICK_RATE_MS);
    }
}

static void Thread3(void *arg) 
{
    /* Thread 3 prints to the console*/
    while (1) 
    {
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}


static void Blink(void *arg) 
{
    int dir = 0;
    while (1) 
    {
        vTaskDelay (500/ portTICK_RATE_MS);
        GPIO_WriteBit(GPIOC , GPIO_Pin_8 , dir ? Bit_SET : Bit_RESET);
        dir = 1 - dir;
    }
}

// Main 
int main(void)
{
    //setup xprintf - I like these func's better than what the book suggests
    //xfunc_input= mygetchar; 
    //xfunc_output= myputchar;

       // set up interrupt priorities for FreeRTOS !!
    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );
    
    /****initialize hardware****/
    // Configure SysTick Timer
    if(SysTick_Config(SystemCoreClock/1000))
    {
        while(1);
    }

    //LEDs
    init_GPIO_pin(GPIOC,GPIO_Pin_8,GPIO_Mode_Out_PP,GPIO_Speed_50MHz);
    init_GPIO_pin(GPIOC,GPIO_Pin_9,GPIO_Mode_Out_PP,GPIO_Speed_50MHz);


    uart_open(USART1,115200);
    puts_rtos("Enter LED on/off to control the state");
    puts_rtos("of the built in LED at pin 9");
    puts_rtos("on the Discovery board \r\n");

    serialMutex = xSemaphoreCreateMutex();

    // Create tasks
    xTaskCreate(Thread1 , // Function to execute
                "Thread 1", // Name
                1024, // Stack size
                NULL, // Parameter (none)
                tskIDLE_PRIORITY + 1 , // Scheduling priority
                NULL // Storage for handle (none)
                );
    
    xTaskCreate(Thread2 , 
                "Thread 2", 
                1024,
                NULL, 
                tskIDLE_PRIORITY + 1 , 
                NULL
                );

    xTaskCreate(Thread3 , 
            "Thread 3", 
            1024,
            NULL, 
            tskIDLE_PRIORITY + 1 , 
            NULL
            );

    xTaskCreate(Blink , 
            "Blink", 
            1024,
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
