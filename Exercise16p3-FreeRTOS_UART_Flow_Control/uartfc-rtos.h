#ifndef UART_H
#define UART_H

/* 
There exists uartfc.h as well, I could have re-used that header for 
the FreeRTOS USART exercise but I'm choosing not to as I've changed
some of the function headers / definitions. While this is all version 
controlled AND each exercise has a folder, it feels cleaner.   

*/

#define QUEUE_SIZE 16
#define HIGH_WATER (QUEUE_SIZE-6) 

int  uart_open (USART_TypeDef * USARTx, uint32_t baud);
    /*
    -Initializes USART/GPIO clocks
    -Configure USART pins
    -Configure and enable the USART1 on STM32
    -Returns a 0 if UART found
    */
int uart_close(USART_TypeDef * USARTx);
    /*
    -Disable the USARTx on STM32
    */

void putchar_rtos(unsigned char c);
/*
Puts char into FreeRTOS Tx queue
*/

unsigned char getchar_rtos(void);
/*
Returns char from FreeRTOS Rx queue
*/

int puts_rtos(char *array);
/* function that calls putchar_rtos until full array is read into the Tx queue.*/

#endif