#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_dma.h>
#include "spi.h"
#include "xprintf.h"
// HEAVILY grabbed from: https://github.com/smarth55/Examples/blob/master/C/AsteroidDodge/spidma.c
// I like their implementation better than what I was trying to do.

// READ
int dmaRcvBytes(SPI_TypeDef *SPIx, void *rbuf, unsigned count, int half) 
{
  DMA_InitTypeDef DMA_InitStructure;
  uint16_t dummy[] = {0xffff};

  DMA_Channel_TypeDef* SPI_DMA_TX;
  DMA_Channel_TypeDef* SPI_DMA_RX;

  // Tabla Rasa
  DMA_DeInit(DMA1_Channel2);
  DMA_DeInit(DMA1_Channel3);
  DMA_DeInit(DMA1_Channel4);
  DMA_DeInit(DMA1_Channel5);

  if(SPIx == SPI1)
  {
    SPI_DMA_TX = DMA1_Channel3;
    SPI_DMA_RX = DMA1_Channel2;
  }

  else if(SPIx == SPI2)
  {
    SPI_DMA_TX = DMA1_Channel5;
    SPI_DMA_RX = DMA1_Channel4;
  }

  else return -1;

  // Common to both channels

  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(SPIx->DR));
  DMA_InitStructure.DMA_PeripheralDataSize = (half) ? DMA_PeripheralDataSize_HalfWord : DMA_PeripheralDataSize_Byte;
  
  DMA_InitStructure.DMA_MemoryDataSize = (half) ? DMA_MemoryDataSize_HalfWord : DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_BufferSize = count;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

  // Rx Channel

  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)rbuf;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;

  DMA_Init(SPI_DMA_RX, &DMA_InitStructure);

  // Tx channel

  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) dummy;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;

  DMA_Init(SPI_DMA_TX, &DMA_InitStructure);

  // Enable channels

  DMA_Cmd(SPI_DMA_RX, ENABLE);
  DMA_Cmd(SPI_DMA_TX, ENABLE);

  // Enable SPI TX/RX request

  SPI_I2S_DMACmd(SPIx, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, ENABLE);

  // Wait for completion

  while (DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET);

  // Disable channels

  DMA_Cmd(SPI_DMA_RX, DISABLE);
  DMA_Cmd(SPI_DMA_TX, DISABLE);

  SPI_I2S_DMACmd(SPIx, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, DISABLE);

  return count;
}

// WRITE
int dmaTxBytes(SPI_TypeDef *SPIx, void *tbuf, unsigned count, int half) {
  DMA_InitTypeDef DMA_InitStructure;
  uint16_t dummy[] = {0xffff};

  DMA_Channel_TypeDef* SPI_DMA_TX;
  DMA_Channel_TypeDef* SPI_DMA_RX;

  // Tabla Rasa
  DMA_DeInit(DMA1_Channel2);
  DMA_DeInit(DMA1_Channel3);
  DMA_DeInit(DMA1_Channel4);
  DMA_DeInit(DMA1_Channel5);

  // Determine SPI DMA path
  if(SPIx == SPI1)
  {
    SPI_DMA_TX = DMA1_Channel3;
    SPI_DMA_RX = DMA1_Channel2;
  }

  else if(SPIx == SPI2)
  {
        xprintf("SPI2 Assigned\r\n"); 
    SPI_DMA_TX = DMA1_Channel5;
    SPI_DMA_RX = DMA1_Channel4;
  }

  else return -1;


  // Common to both channels

  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(SPIx->DR));
  DMA_InitStructure.DMA_PeripheralDataSize = (half) ? DMA_PeripheralDataSize_HalfWord : DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = (half) ? DMA_MemoryDataSize_HalfWord : DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_BufferSize = count;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

  // Rx Channel

  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) dummy;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;

  DMA_Init(SPI_DMA_RX, &DMA_InitStructure);

  // Tx channel

  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)tbuf;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;

  DMA_Init(SPI_DMA_TX, &DMA_InitStructure);

  // Enable channels

  DMA_Cmd(SPI_DMA_TX, ENABLE);
  DMA_Cmd(SPI_DMA_RX, ENABLE);

  // Enable SPI TX/RX request

  SPI_I2S_DMACmd(SPIx, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, ENABLE);

  // Wait for completion
  xprintf("waiting on TC4 Flag\r\n");
  while (DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET);
  xprintf("Flag Resolved!\r\n");
  // Disable channels

  DMA_Cmd(SPI_DMA_RX, DISABLE);
  DMA_Cmd(SPI_DMA_TX, DISABLE);

  SPI_I2S_DMACmd(SPIx, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, DISABLE);

  return count;
}

// EXCHANGE
int dmaExgBytes(SPI_TypeDef *SPIx, void *rbuf, void *tbuf, unsigned count, int half)
{
  DMA_InitTypeDef DMA_InitStructure;
  uint16_t dummy[] = {0xffff};

  DMA_Channel_TypeDef* SPI_DMA_TX;
  DMA_Channel_TypeDef* SPI_DMA_RX;

  // Tabla Rasa
  DMA_DeInit(DMA1_Channel2);
  DMA_DeInit(DMA1_Channel3);
  DMA_DeInit(DMA1_Channel4);
  DMA_DeInit(DMA1_Channel5);

  //Determine SPI DMA path
  if(SPIx == SPI1)
  {
    SPI_DMA_TX = DMA1_Channel3;
    SPI_DMA_RX = DMA1_Channel2;
  }

  else if(SPIx == SPI2)
  {
    SPI_DMA_TX = DMA1_Channel5;
    SPI_DMA_RX = DMA1_Channel4;
  }

  else return -1;

  // Common to both channels

  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(SPIx->DR));
  DMA_InitStructure.DMA_PeripheralDataSize = (half) ? DMA_PeripheralDataSize_HalfWord : DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = (half) ? DMA_MemoryDataSize_HalfWord : DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_BufferSize = count;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

  // Rx Channel

  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)rbuf;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;

  DMA_Init(SPI_DMA_RX, &DMA_InitStructure);

  // Tx channel

  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)tbuf;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;

  DMA_Init(SPI_DMA_TX, &DMA_InitStructure);

  // Enable channels

  DMA_Cmd(SPI_DMA_RX, ENABLE);
  DMA_Cmd(SPI_DMA_TX, ENABLE);

  // Enable SPI TX/RX request

  SPI_I2S_DMACmd(SPIx, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, ENABLE);

  // Wait for completion

  while (DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET);

  // Disable channels

  DMA_Cmd(SPI_DMA_RX, DISABLE);
  DMA_Cmd(SPI_DMA_TX, DISABLE);

  SPI_I2S_DMACmd(SPIx, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, DISABLE);

  return count;
}


int xchng_datablock(SPI_TypeDef *SPIx, int half, const void *tbuf, void *rbuf, unsigned count) 
{
  if (count > 4) { 
    xprintf("count>4\r\n"); 
    if (half) 
    {
      SPI_DataSizeConfig(SPIx, SPI_DataSize_16b);
    }
    if (!tbuf) 
    {
      dmaRcvBytes(SPIx, rbuf, count, half);
    } 
    else if (!rbuf) 
    {
      xprintf("in DMA Tx!\r\n"); 
      dmaTxBytes(SPIx, tbuf, count, half);
    } 
    else 
    {
            xprintf("in DMA Rx!\r\n"); 
      dmaExgBytes(SPIx, rbuf, tbuf, count, half);
    }
    SPI_DataSizeConfig(SPIx, SPI_DataSize_8b);
  } 
  else 
  {
    xprintf("count<=4\r\n"); 
    if (half) 
    {
      spiReadWrite16(SPIx, rbuf, tbuf, count, SPI_FAST);
    } 
    else 
    {
      spiReadWrite(SPIx , rbuf, tbuf ,count , SPI_FAST);
    }
  }
}