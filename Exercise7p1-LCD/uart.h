#ifndef UART_H
#define UART_H

int uart_open(USART_TypeDef* USARTx , uint32_t baud); 
    /*
    -Initializes USART/GPIO clocks
    -Configure USART pins
    -Configure and enable the USART1 on STM32
    -Returns a 0 if UART found
    */
int uart_close(USART_TypeDef* USARTx);
    /*
    -Disable the USARTx on STM32
    */
int uart_putc(int c, USART_TypeDef* USARTx);
    /*
    -sends a character to USART (w/mask for lower 8 bits)
    */
int uart_getc(USART_TypeDef* USARTx);
    /*
    -returns USART data register's lower 8 bits
    */

int uart_putstring(char *array,USART_TypeDef* USARTx);
/*
-uses uart_putc w/ a loop to print an array to UART
-returns 0 if successful like uart_putc
*/
#endif