#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_usart.h>
#include <misc.h>


/****FREERTOS INCLUDES****/
#include "FreeRTOSConfig.h"
#include "FreeRTOS_Source/include/FreeRTOS.h"
#include "FreeRTOS_Source/include/FreeRTOS.h"
#include "FreeRTOS_Source/include/task.h"
#include "FreeRTOS_Source/include/queue.h"
#include "FreeRTOS_Source/include/list.h"
#include "FREERTOS_Source/include/timers.h"
#include "FreeRTOS_Source/include/semphr.h"

/****HEADER****/
#include "uartfc-rtos.h"

int RxOverflow = 0;

// TxPrimed is used to signal that Tx send buffer needs to be primed
// to commence sending -- it is cleared by the IRQ, set by uart_write

static int TxPrimed = 0;

QueueHandle_t UART1_TXq, UART1_RXq;

int uart_open (uint8_t uart, uint32_t baud, uint32_t flags)
{
  USART_InitTypeDef USART_InitStructure; 
  GPIO_InitTypeDef GPIO_InitStructure; 
  NVIC_InitTypeDef NVIC_InitStructure;

  if (uart == 1) {
    
    // get things to a known state

    USART_DeInit(USART1);

    // Turn on clocks

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA  |
			   RCC_APB2Periph_AFIO |
			   RCC_APB2Periph_USART1,
			   ENABLE);

    // Configure TX pin

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Configure RX pin

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Configure CTS pin

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Configure RTS pin -- software controlled

    GPIO_WriteBit(GPIOA, GPIO_Pin_12, 1);          // nRTS disabled
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Configure NVIC

    /* Configure the NVIC Preemption Priority Bits */  
    /*This should be PriorityGroup_4 to be congruent w/ FreeRTOS*/
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  
    /* Enable the USART1 Interrupt */

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    /* ***IMPORTANT***
    This must be >5 (RTOS preemption level) or 
    you you get an error. This took forever to debug
    The book states this but its difficult to wrap your 
    head around at first. I learned the hard way*/
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6; 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Create Queues
    UART1_RXq = xQueueCreate(QUEUE_SIZE,sizeof(char));
    UART1_TXq = xQueueCreate(QUEUE_SIZE,sizeof(char));

    // Configure the UART

    USART_StructInit(&USART_InitStructure); 
    USART_InitStructure.USART_BaudRate = baud;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_CTS;
    USART_InitStructure.USART_Mode  = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1,&USART_InitStructure); 


    // Enable RX Interrupt.  TX interrupt enabled in send routine

    USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);


    // Enable Usart1

    USART_Cmd(USART1, ENABLE); 

    // nRTS enabled

    GPIO_WriteBit(GPIOA, GPIO_Pin_12, 0);          

    return 0;
  } 
  return 1;  // only handle UART1
}


int uart_close(uint8_t uart)
{

}

int uart_putc_rtos(int c)
{
    xQueueSend(UART1_TXq , &c, portMAX_DELAY);
    // kick the transmitter interrupt
    USART_ITConfig(USART1 , USART_IT_TXE , ENABLE);
    return 0;
}

int uart_getc_rtos()
{
    int8_t buf;
    xQueueReceive(UART1_RXq , &buf , portMAX_DELAY);
    return buf;
}

void USART1_IRQHandler(void)
{
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
  
  if(USART_GetITStatus(USART1 , USART_IT_RXNE) != RESET)
  {
    uint8_t data;
    
    USART_ClearITPendingBit(USART1 , USART_IT_RXNE);
    
    data = USART_ReceiveData(USART1) & 0xff;
    if (xQueueSendFromISR(UART1_RXq , &data,!&xHigherPriorityTaskWoken) != pdTRUE)
      RxOverflow = 1;
  }
  if(USART_GetITStatus(USART1 , USART_IT_TXE) != RESET) 
  {
    uint8_t data;
    if (xQueueReceiveFromISR(UART1_TXq , &data,!&xHigherPriorityTaskWoken) == pdTRUE)
    {
      USART_SendData(USART1 , data);
    }
    else 
    {
      // turn off interrupt
      USART_ITConfig(USART1 , USART_IT_TXE , DISABLE);
    }
  }
  // Cause a scheduling operation if necessary
  portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}