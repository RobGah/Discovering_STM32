/****STM32 Specifics****/
#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_usart.h>

/****Standard lib stuff****/
#include <misc.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/types.h>

/****Header****/
#include "uartfc.h"

int RxOverflow = 0;

// TxPrimed is used to signal that Tx send buffer needs to be primed
// to commence sending -- it is cleared by the IRQ, set by uart_write

static int TxPrimed = 0;

struct Queue {
  //conceptually I'm thinking of a clockwise moving circle
  //read "chases" write as the author explains. 
  uint16_t pRD, pWR; //read and write "pointers" - more like indexes
  uint8_t  q[QUEUE_SIZE]; //actual buffer of some size
};

static struct Queue UART1_TXq, UART1_RXq; //instantiate 2 circular buffers.

static int QueueFull(struct Queue *q)
{
  //to think it through:
  //if write pointer is at last position in buffer and read is at the 1st position.
  //e.g. if 12 position buffer [0-11] and we get pWR+1=12, then pWR+1/QUEUE_SIZE=0.
  //Also accounts for circular nature. If 12 position buffer and pWR+1=6, returns 6, pWR is at 5 
  //and we're full if pRD is at 6 ("beginning" of our circle) 
  return (((q->pWR + 1) % QUEUE_SIZE) == q->pRD); //1 if we're full
}

static int QueueEmpty(struct Queue *q)
{
  return (q->pWR == q->pRD); //if read and write are at same location - buffer is empty. 
}

static int QueueAvail(struct Queue *q)
{
  //Looks more like "QueueFilled" - returns # of occupied slots
  // gives total size + write pointer location - read pointer location
  //e.g. if size is 12, write is at 3 and read is at 8
  // 12+3-8 = 7, 7 mod 12 = 7 slots full. aka 8,9,10,11,12,1,2 are all full
  //returns 0 if WR==RD, indicating totally empty.
  return (QUEUE_SIZE + q->pWR - q->pRD) % QUEUE_SIZE;
}

static int Enqueue(struct Queue *q, const uint8_t *data, uint16_t len)
{
  int i;
  //while we're not full AND we still have data to load...
  for (i = 0; !QueueFull(q) && (i < len); i++)
    {
      q->q[q->pWR] = data[i]; //load up data at the current location of the write pointer
      //if we're at the end of the buffer, reset pWR to 0 (completing the circle!)
      //else, advance the write pointer to next empty segment of the buffer. 
      q->pWR = ((q->pWR + 1) ==  QUEUE_SIZE) ? 0 : q->pWR + 1; 
    }
  return i;//returns # of data items loaded
}

static int Dequeue(struct Queue *q, uint8_t *data, uint16_t len)
{
  int i;
  for (i = 0; !QueueEmpty(q) && (i < len); i++)
    {
      data[i] = q->q[q->pRD]; //read data from the pRD "pointer" (index) location
      //if pRD is at the last position in the queue, wraparound to 0
      //else, increment pRD
      q->pRD = ((q->pRD + 1) ==  QUEUE_SIZE) ? 0 : q->pRD + 1;
    }
  return i; //returns # of items read. 
}

int  uart_open (uint8_t uart, uint32_t baud, uint32_t flags)
{

  //n.b. pin assignment is the same for the Blue Pill. 
  //flags as a param is a dummy?

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


int uart_close(uint8_t uart)
{
  //nothing here?
  if(uart==1) //only handle UART1 as with uart_open
  {
    USART_Cmd(USART1,DISABLE);
  }

}

ssize_t uart_write(uint8_t uart, const uint8_t *buf, size_t nbyte)
//signed size_t - interesting datatype. size_t is garunteed to be big enough to handle
//the largest element the system can handle. Its the return type of sizeof(). Love it. 
//It seems to be "unsigned int" for me as per its typedef
//ssize_t can take on a negative value. 
{
  uint8_t data;
  int i = 0;
  //if USART1 and we have bytes to write
  if (uart == 1 && nbyte) 
    {
      i = Enqueue(&UART1_TXq, buf, nbyte); //queue them up for sending
  
      // if we added something and the Transmitter isn't working
      // give it a kick by turning on the buffer empty interrupt

      if (!TxPrimed)
	{
	  TxPrimed = 1;

	  // This implementation guarantees that USART_IT_Config
	  // is not called simultaneously in the interrupt handler and here.

	  USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
	}
    }
  return i;
}

ssize_t uart_read (uint8_t uart, uint8_t *buf, size_t nbyte)
{
  int i = 0;

  if (uart == 1)
    {

      i = Dequeue(&UART1_RXq, buf, nbyte); //count # of things read

      // If the queue has fallen below high water mark, enable nRTS

      if (QueueAvail(&UART1_RXq) <= HIGH_WATER)
	GPIO_WriteBit(GPIOA, GPIO_Pin_12, 0); //tell our usart to send!
    }
  return i;
}

void USART1_IRQHandler(void)
{
  if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
      uint8_t  data;

      // clear the interrupt

      USART_ClearITPendingBit(USART1, USART_IT_RXNE);

      // buffer the data (or toss it if there's no room) 
      // Flow control is supposed to prevent this

      data = USART_ReceiveData(USART1) & 0xff; //get Rec'd data and all 1's mask

      //if we can't enqueue our rec'd byte of data into the recieve register
      if (!Enqueue(&UART1_RXq, &data, 1))
	RxOverflow = 1; //set overflow flag

      // If queue is above high water mark, disable nRTS

      if (QueueAvail(&UART1_RXq) > HIGH_WATER)
	GPIO_WriteBit(GPIOA, GPIO_Pin_12, 1);   //tells usart to stop sending
    }
  
  if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
    {   
      /* Write one byte to the transmit data register */

      uint8_t data;

      if (Dequeue(&UART1_TXq, &data, 1)) //grab a byte from the tx register
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
}

char getchar(void)
//stdio gives us size_t support, but stipulates
{
  uint8_t data;
  while (Dequeue (&UART1_RXq , &data , 1) != 1);
      // If the queue has fallen below high water mark , enable nRTS
  if (QueueAvail (& UART1_RXq) <= HIGH_WATER)
    GPIO_WriteBit(GPIOA , GPIO_Pin_12 , 0);
  return data;
}

int putchar(int c)
{
  while (! Enqueue (&UART1_TXq , c, 1));
    if (! TxPrimed) {
      TxPrimed = 1;
      USART_ITConfig(USART1 , USART_IT_TXE , ENABLE);
    }
}