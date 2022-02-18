#ifndef UART_H
#define UART_H

/* 
There exists uartfc.h as well, I could have re-used that header for 
the FreeRTOS USART exercise but I'm choosing not to as I've changed
some of the function headers / definitions. While this is all version 
controlled AND each exercise has a folder, it feels cleaner.   

*/

#define QUEUE_SIZE 12
#define HIGH_WATER (QUEUE_SIZE-6) 

int  uart_open (uint8_t uart, uint32_t baud,uint32_t flags);
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

int uart_putc_rtos(int c);
/*
Puts char into FreeRTOS Tx queue
*/

int uart_getc_rtos();
/*
Returns char from FreeRTOS Rx queue
*/

#endif