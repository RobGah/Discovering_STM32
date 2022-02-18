/****STM32 Specifics****/
#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_usart.h>

/****Standard lib stuff****/
#include <misc.h>
//#include <stdio.h>
//#include <stddef.h>
//#include <sys/types.h>
//#include <string.h>
 
/****FREERTOS INCLUDES****/
#include "FreeRTOSConfig.h"
#include "FreeRTOS_Source/include/FreeRTOS.h"
#include "FreeRTOS_Source/include/FreeRTOS.h"
#include "FreeRTOS_Source/include/task.h"
#include "FreeRTOS_Source/include/queue.h"
#include "FreeRTOS_Source/include/list.h"
#include "FREERTOS_Source/include/timers.h"
#include "FreeRTOS_Source/include/semphr.h"


/****Header****/
//#include "uartfc-rtos.h"

static volatile int RxOverflow = 0;

// TxPrimed is used to signal that Tx send buffer needs to be primed
// to commence sending -- it is cleared by the IRQ, set by uart_write
// volatile as its operated on by both the ISR and possibly multiple threads
static volatile int TxPrimed = 0;


/*** PUBLIC FUNCTIONS***/

//instantiate 2 FREERTOS Queues w/ global scope.
QueueHandle_t UART1_TXq, UART1_RXq; 

int  uart_open (USART_TypeDef * USARTx, uint32_t baud)
{

  // create our queues

  UART1_TXq = xQueueCreate(QUEUE_SIZE,sizeof(char));
  UART1_RXq = xQueueCreate(QUEUE_SIZE,sizeof(char));

  while (UART1_TXq == NULL || UART1_TXq == NULL)
  {
    UART1_TXq = xQueueCreate(QUEUE_SIZE,sizeof(char));
    vTaskDelay(100);
    UART1_RXq = xQueueCreate(QUEUE_SIZE,sizeof(char));
    vTaskDelay(100);
  }

  //n.b. pin assignment is the same for the Blue Pill. 

  USART_InitTypeDef USART_InitStructure; 
  GPIO_InitTypeDef GPIO_InitStructure; 
  NVIC_InitTypeDef NVIC_InitStructure;

  if (USARTx == USART1) {
    
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

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  
    /* Enable the USART1 Interrupt */

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);


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


int uart_close(USART_TypeDef * USARTx)
{
  //nothing here?
  if(USARTx==USART1) //only handle UART1 as with uart_open
  {
    USART_Cmd(USART1,DISABLE);
  }

}

unsigned char getchar_rtos(void)
{
  // 0xFF will be returned if the RX queue is empty
  // getchar shouldn't be called then OR 0xFF can be
  // checked for by the calling routine
  if(uxQueueMessagesWaiting(UART1_RXq) == 0)
  {
    return 0xFF;
  }
  uint8_t data; 
  // Don't block for more than 10 ticks
  while(xQueueReceive(UART1_RXq , (void *) &data , portMAX_DELAY) != pdTRUE);
  // If the queue has fallen below high water mark , enable nRTS
  if (uxQueueMessagesWaiting(UART1_RXq) <= HIGH_WATER)
  {
    GPIO_WriteBit(GPIOA , GPIO_Pin_12 , 0);
  }
  return data;
  
}

void putchar_rtos(unsigned char c)
{
  // if we are successful in queueing the data
  while(xQueueSend(UART1_TXq , (void *) &c, portMAX_DELAY) != pdTRUE);
  if (! TxPrimed) 
  {
    TxPrimed = 1;
    USART_ITConfig(USART1 , USART_IT_TXE , ENABLE);
  }
}

int puts_rtos(char *array)
{
    for(int i = 0; i<sizeof(array)/sizeof(char);++i)
        {
            putchar_rtos(array[i]);
        }
    //auto carriage + newline after string
    //putchar_rtos('\r'); 
    //putchar_rtos('\n'); 
    return(0);
}


void USART1_IRQHandler(void)
{
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
  
  if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
      
      uint8_t  data;

      // clear the interrupt

      USART_ClearITPendingBit(USART1, USART_IT_RXNE);

      // buffer the data (or toss it if there's no room) 
      // Flow control is supposed to prevent this

      data = USART_ReceiveData(USART1) & 0xff; //get Rec'd data and all 1's mask

      //if we can't enqueue our rec'd byte of data into the recieve register
      if (xQueueSendFromISR(UART1_RXq,(void *) &data,&xHigherPriorityTaskWoken)!=pdTRUE)
	      {
        RxOverflow = 1; //set overflow flag
        }
      // If queue is above high water mark, disable nRTS

      if (uxQueueMessagesWaiting(UART1_RXq) > HIGH_WATER)
      {
	      GPIO_WriteBit(GPIOA, GPIO_Pin_12, 1);   //tells usart to stop sending
      }
    }
  
  if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
    {   
      /* Write one byte to the transmit data register */

      uint8_t data;

      //grab a byte from the tx register
      if (xQueueReceiveFromISR(UART1_TXq, (void *) &data, &xHigherPriorityTaskWoken)==pdTRUE) 
	    {
	      USART_SendData(USART1, data); //send it!
	    }
      else
	    {
	      // if we have nothing to send, disable the interrupt
	      // and wait for a kick
	      USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
	      TxPrimed = 0;
	    }
    }
    // Cause a scheduling operation if necessary
    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}

