#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_spi.h>
#include "spi.h"

/*
Setup:

SPI lines are connected in loopback mode (MISO tied to MOSI)
SS is arbitrarily PA8. 

Ought to see the same data pattern on MOSI as MISO w/
a logic analyzer. 

Author has implemented an incrementing message for both 8 and
16 bit SPI modes.
*/


static uint8_t txbuf[4], rxbuf[4];
static uint16_t txbuf16[4], rxbuf16[4];
static int i, j;

void csInit(void);

int main()
{
    csInit(); //PA8 CS initialization
    spiInit(SPI2);

    //fill and xmit txbuf with 0-32 in 4 byte steps
    for(i=0; i<8; ++i)
    {
        for(j = 0; j < 4; ++j)
        {
            txbuf[j] = i*4 + j;
        }
        GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_RESET); 
        spiReadWrite(SPI2, rxbuf, txbuf, 4, SPI_SLOW);
        GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_SET);
        for(j = 0; j < 4; ++j)
        {   
            //if something fails in loopback mode
            if (rxbuf[j] != txbuf[j]) 
            {
                assert_failed(__FILE__, __LINE__);
            }
            
        }
    }
    for (i = 0; i < 8; ++i);
    {
        for(j = 0; j < 4; ++j)
        {
            txbuf16[j] = i*4 + j + (i<<8); 
            //author shifts bits past the 8th bit just 
            //to prove we're in 16bit mode?
        }
        GPIO_WriteBit(GPIOB,GPIO_Pin_7,Bit_RESET);
        spiReadWrite16(SPI2 ,rxbuf16, txbuf16, 4, SPI_SLOW);
        GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_SET);
        for(j = 0; j < 4; ++j)
        {
            if(rxbuf16[j] != txbuf16[j])
            {
                assert_failed(__FILE__, __LINE__);
            }
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

//CS pin init
void csInit(void)
{
    //clock for GPIOC
    RCC_AHBPeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);

    //CS pin setup
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);
    //pin specs
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&GPIO_InitStructure);
    GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_SET);
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

