#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_dma.h>
#include "spi.h"

// Grabbed from: https://github.com/smarth55/Examples/blob/master/C/AsteroidDodge/spidma.c

int xchng_datablock(SPI_TypeDef *SPIx, int half, const void *tbuf, void *rbuf, unsigned count) {
  if (count > 4) {  
    if (half) SPI_DataSizeConfig(SPI2, SPI_DataSize_16b);
    if (!tbuf) {
      dmaRcvBytes(SPI2, rbuf, count, half);
    } else if (!rbuf) {
      dmaTxBytes(SPI2, tbuf, count, half);
    } else {
      dmaExgBytes(SPI2, rbuf, tbuf, count, half);
    }
    SPI_DataSizeConfig(SPI2, SPI_DataSize_8b);
  } else {
    if (half) {
      spiReadWrite16(SPI2, rbuf, tbuf, count, SPI_FAST);
    } else {
      spiReadWrite(SPI2 , rbuf, tbuf ,count , SPI_FAST);
    }
  }
}

// READ (done)
int dmaRcvBytes(SPI_TypeDef *SPIx, void *rbuf, unsigned count, int half) {
  DMA_InitTypeDef DMA_InitStructure;
  uint16_t dummy[] = {0xffff};

  DMA_DeInit(DMA1_Channel4);
  DMA_DeInit(DMA1_Channel5);

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

  DMA_Init(DMA1_Channel4, &DMA_InitStructure);

  // Tx channel

  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) dummy;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;

  DMA_Init(DMA1_Channel5, &DMA_InitStructure);

  // Enable channels

  DMA_Cmd(DMA1_Channel4, ENABLE);
  DMA_Cmd(DMA1_Channel5, ENABLE);

  // Enable SPI TX/RX request

  SPI_I2S_DMACmd(SPIx, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, ENABLE);

  // Wait for completion

  while (DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET);

  // Disable channels

  DMA_Cmd(DMA1_Channel4, DISABLE);
  DMA_Cmd(DMA1_Channel5, DISABLE);

  SPI_I2S_DMACmd(SPIx, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, DISABLE);

  return count;
}

// WRITE
int dmaTxBytes(SPI_TypeDef *SPIx, void *tbuf, unsigned count, int half) {
  DMA_InitTypeDef DMA_InitStructure;
  uint16_t dummy[] = {0xffff};

  DMA_DeInit(DMA1_Channel4);
  DMA_DeInit(DMA1_Channel5);

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

  DMA_Init(DMA1_Channel4, &DMA_InitStructure);

  // Tx channel

  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)tbuf;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;

  DMA_Init(DMA1_Channel5, &DMA_InitStructure);

  // Enable channels

  DMA_Cmd(DMA1_Channel4, ENABLE);
  DMA_Cmd(DMA1_Channel5, ENABLE);

  // Enable SPI TX/RX request

  SPI_I2S_DMACmd(SPIx, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, ENABLE);

  // Wait for completion

  while (DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET);

  // Disable channels

  DMA_Cmd(DMA1_Channel4, DISABLE);
  DMA_Cmd(DMA1_Channel5, DISABLE);

  SPI_I2S_DMACmd(SPIx, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, DISABLE);

  return count;
}

// EXCHANGE
int dmaExgBytes(SPI_TypeDef *SPIx, void *rbuf, void *tbuf, unsigned count, int half) {
  DMA_InitTypeDef DMA_InitStructure;
  uint16_t dummy[] = {0xffff};

  DMA_DeInit(DMA1_Channel4);
  DMA_DeInit(DMA1_Channel5);

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

  DMA_Init(DMA1_Channel4, &DMA_InitStructure);

  // Tx channel

  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)tbuf;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;

  DMA_Init(DMA1_Channel5, &DMA_InitStructure);

  // Enable channels

  DMA_Cmd(DMA1_Channel4, ENABLE);
  DMA_Cmd(DMA1_Channel5, ENABLE);

  // Enable SPI TX/RX request

  SPI_I2S_DMACmd(SPIx, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, ENABLE);

  // Wait for completion

  while (DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET);

  // Disable channels

  DMA_Cmd(DMA1_Channel4, DISABLE);
  DMA_Cmd(DMA1_Channel5, DISABLE);

  SPI_I2S_DMACmd(SPIx, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, DISABLE);

  return count;
}