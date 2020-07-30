#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_usart.h>
#include "uart.h"
#include "spi.h"
#include "eeprom.h"
#include <string.h>

/*
Setup:

25LC160 is connected to the STM32 "Blue Pill" by way of:

EEPROM  BluePill    Function
1       PB12        CS
2       PB14        SlaveOut
3       3V          WriteProtect
4       GND         VSS
5       PB15        SlaveIn
6       PB13        Clock
7       3V          HOLD
8       3V          VCC

To test read/write:
-Write in "The STM32 Says Hello World!" into the EEPROM
-Perform a read operation at the address written to
-Send the read message over UART to confirm

UART    BluePill    BluePill Pin 
TXD     RXD         A9
RXD     TXD         A10
GND     GND         GND
5V      5V          5V

*/

#define USE_FULL_ASSERT

uint8_t message[] = "The STM32 Says Hello World!\n";
uint8_t recieved_from_memory[sizeof(message)];

uint16_t eeprom_address = 0x403; //1027, in the middle of memory. Falls in the middle of a page for testing

void Delay(uint32_t nTime);

int main()
{
    
    eepromInit(); //initializes SPI for us

    //eepromWrite((uint8_t*) &message, sizeof(message) - 1, eeprom_address); 
    eepromWrite(message, sizeof(message)-1, eeprom_address);
    
    eepromRead(recieved_from_memory, sizeof(recieved_from_memory), eeprom_address);

    uart_open(USART1,9600);
    // Get onboard LED initialized.
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
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

        if(strcmp(message,recieved_from_memory) != 0)
        {
            char error[]  = "nope!\n";
            for(int i = 0; i<strlen(error);++i)
            {
                uart_putc(error[i],USART1);
            }
        }

        for(int i = 0; i<strlen(recieved_from_memory); ++i)
        {
        uart_putc(recieved_from_memory[i], USART1);
        GPIO_WriteBit(GPIOC, GPIO_Pin_13, (ledval) ? Bit_SET : Bit_RESET);
        ledval = 1-ledval;
            //uart_putc('a', USART1);
            Delay (250); // wait 250ms
        }
        //uart_putc('/n',USART1); //newline
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

