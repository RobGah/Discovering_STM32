#ifndef UART_H
#define UART_H

#define QUEUE_SIZE 12
#define HIGH_WATER (QUEUE_SIZE-6) //arbitrary and eventually empirical value

int  uart_open (uint8_t uart, uint32_t baud, uint32_t flags);
    /*
    -Initializes USART/GPIO clocks
    -Configure USART pins
    -Configure and enable the USART1 on STM32
    -Returns a 0 if UART found
    */
int uart_close(uint8_t uart);
    /*
    -Disable the USARTx on STM32
    */

ssize_t uart_write(uint8_t uart, const uint8_t *buf, size_t nbyte);
//writes a data buffer to the tx circular buffer by way of Enqueue()

ssize_t uart_read (uint8_t uart, uint8_t *buf, size_t nbyte);
//reads from uart recieve circular buffer by way of Dequeue() 

//////////////////////////////////////////////////////////////////////////////////////
/********* BELOW ARE DEPRECIATED UNTIL I GET TO CODING UP SOME PREPROCESSOR STATEMENTS 
    TO SELECT BETWEEN BASIC UART AND FLOW-CONTROL UART**********/
/////////////////////////////////////////////////////////////////////////////////////

//int uart_putc(int c, USART_TypeDef* USARTx);
    /*
    -sends a character to USART (w/mask for lower 8 bits)
    */
//int uart_getc(USART_TypeDef* USARTx);
    /*
    -returns USART data register's lower 8 bits
    */

//int uart_puts(char *array,USART_TypeDef* USARTx);
/*
-uses uart_putc w/ a loop to print an array to UART
-returns 0 if successful like uart_putc
*/
#endif