#ifndef _UART_H
#define _UART_H

#define QUEUE_SIZE = 12
#define HIGH_WATER (QUEUE_SIZE-6)

int uart_open(USART_TypeDef* USARTx, uint32_t baud, uint32_t flags);
int uart_close(USART_TypeDef* USARTx);
int uart_putc(int c, USART_TypeDef* USARTx);
int uart_getc(USART_TypeDef* USARTx);

#endif
