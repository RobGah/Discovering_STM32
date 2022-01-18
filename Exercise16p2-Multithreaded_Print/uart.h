#ifndef UART_H
#define UART_H

int uart_open(USART_TypeDef* USARTx, uint32_t baud);
int uart_close(USART_TypeDef* USARTx);


//////////////////////////////////////////////////////////////////////////////////////
/********* BELOW ARE DEPRECIATED UNTIL I GET TO CODING UP SOME PREPROCESSOR STATEMENTS 
    TO SELECT BETWEEN BASIC UART AND FLOW-CONTROL UART**********/
/////////////////////////////////////////////////////////////////////////////////////



int uart_putc(int c, USART_TypeDef* USARTx);
    /*
    -sends a character to USART (w/mask for lower 8 bits)
    */
int uart_getc(USART_TypeDef* USARTx);
    /*
    -returns USART data register's lower 8 bits
    */

int uart_puts(char *array,USART_TypeDef* USARTx);
/*
-uses uart_putc w/ a loop to print an array to UART
-returns 0 if successful like uart_putc
*/
#endif