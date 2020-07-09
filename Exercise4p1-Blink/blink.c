#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>

void Delay(uint32_t nTime);

int main(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    // Enable Peripheral Clocks
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
    // Configure Pins
    GPIO_StructInit(&GPIO_InitStructure);
    //GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    //for use with Blue Pill, built-in LED is 13
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    //"replace GPIOC w/ 66 when initializing an debug w/ gdb"
    //GPIO_Init(66, &GPIO_InitStructure);
    /*Program received signal SIGTRAP, Trace/breakpoint trap.
    GPIO_Init (GPIOx=0x20001fec, GPIO_InitStruct=0x40012000)
    at ../Library/stm32f10x_gpio.c:179
    179       assert_param(IS_GPIO_MODE(GPIO_InitStruct->GPIO_Mode));
    (gdb) Continuing.

    I got that by merely putting a breakpoint at main. 
    */

// Configure SysTick Timer
    if(SysTick_Config(SystemCoreClock/1000))
    {
        while(1);
    }
    while (1) 
    {
        //toggle LED
        static int ledval = 0;
        //For use w/ Blue Pill, GPIO pin 13 is built-in LED
        GPIO_WriteBit(GPIOC, GPIO_Pin_13, (ledval) ? Bit_SET : Bit_RESET);
        //GPIO_WriteBit(GPIOC, GPIO_Pin_9, (ledval) ? Bit_SET : Bit_RESET);
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