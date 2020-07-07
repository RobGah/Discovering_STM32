#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_usart.h>
#include "uart.h"

void Delay(uint32_t nTime);
int uart_open(USART_TypeDef * USARTx, uint32_t baud);

int main(void)
{
    uart_open(USART1, 9600);

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
        uart_putc('Hello World\n\r', USART1);
        GPIO_WriteBit(GPIOC, GPIO_Pin_13, (ledval) ? Bit_SET : Bit_RESET);
        ledval = 1-ledval;
        Delay (250); // wait 250ms
    }


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

