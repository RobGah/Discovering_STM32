#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_usart.h>
#include <string.h>
#include "uart.h"

int uart_open(USART_TypeDef* USARTx, uint32_t baud)//book calls for uint32_t flags as a param but theres no need?
{
    /*
    -Initializes USART/GPIO clocks
    -Configure USART pins
    -Configure and enable the USART1 on STM32
    -Returns a 1 if UART found, 0 if it fails to find UART specified
    */

    //instantiate and initialize the GPIO structure
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_StructInit(&GPIO_InitStruct);


    if(USARTx == USART1)
    {
        //Clock init
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA, ENABLE);

        //USART1 TX init
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; 
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_Init(GPIOA, &GPIO_InitStruct);
    
        //USART1 RX init
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOA,&GPIO_InitStruct);
    }

    else if(USARTx == USART2)
    {
        //Clock init
        RCC_APB2PeriphClockCmd(RCC_APB1Periph_USART2 | RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA, ENABLE);

        //USART2 TX init
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_Init(GPIOA, &GPIO_InitStruct);
    
        //USART2 RX init
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOA,&GPIO_InitStruct);   
    }
    else if(USARTx == USART3)
    {
        //Clock init
        RCC_APB2PeriphClockCmd(RCC_APB1Periph_USART3 | RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB, ENABLE);

        //USART3 TX init
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_Init(GPIOA, &GPIO_InitStruct);
    
        //USART3 RX init
        GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOA,&GPIO_InitStruct);  
    }

    else
    {
        return -1;
    }
    
    
    //Initialize and Enable USART
    USART_InitTypeDef USART_InitStructure;
    USART_StructInit(&USART_InitStructure);
    USART_InitStructure.USART_BaudRate = baud;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USARTx,&USART_InitStructure);
    USART_Cmd(USARTx,ENABLE);

    return(0); //all good
}

int uart_close(USART_TypeDef* USARTx)
{
    USART_Cmd(USARTx,DISABLE); //I guess?
}

int uart_putc(int c, USART_TypeDef* USARTx)
{
    while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET );
    USARTx->DR = (c & 0xff); 
    return(0);
}

int uart_getc(USART_TypeDef* USARTx)
{
    while(USART_GetFlagStatus(USARTx, USART_FLAG_RXNE) == RESET);
    return(USARTx->DR & 0xff); 
}

int uart_puts(char *array,USART_TypeDef* USARTx)
{
    for(int i = 0; i<strlen(array);++i)
        {
            uart_putc(array[i],USARTx);
        }
    
    uart_putc('\r\n',USARTx); //auto newline after string 
    return(0);
}