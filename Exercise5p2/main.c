#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include "uart.h"
#include <string.h>

void Delay(uint32_t nTime);
//int uart_open(USART_TypeDef * USARTx, uint32_t baud);
//int uart_putc(int c, USART_TypeDef* USARTx);

char string_to_send[]  = "Hello World!\n\r";

int main(void)
{
    //Initialize UART
    uart_open(USART1, 9600);

// Get onboard LED initialized.
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

// Configure SysTick Timer
    if(SysTick_Config(SystemCoreClock/1000))
    {
        while(1);
    }
    while (1) 
    {
        //toggle LED for sign of life while writing to USART1
        static int ledval = 0;
        
        //For use w/ Blue Pill, GPIO pin 13 is built-in LED
        for(int i = 0; i<strlen(string_to_send); ++i)
        {
        uart_putc(string_to_send[i], USART1);
        GPIO_WriteBit(GPIOC, GPIO_Pin_13, (ledval) ? Bit_SET : Bit_RESET);
        ledval = 1-ledval;
            //uart_putc('a', USART1);
            Delay (250); // wait 250ms
        }
    }
return(0);
}
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
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file , uint32_t line)
{
    /* Infinite loop */
    /* Use GDB to find out why we're here */
    while (1);
}
#endif

