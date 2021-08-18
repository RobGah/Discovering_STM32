#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_dma.h>
#include "spi.h"

#define SPIDMA

void spiInit(SPI_TypeDef *SPIx)
{
    SPI_InitTypeDef SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_StructInit(&GPIO_InitStructure);
    SPI_StructInit(&SPI_InitStructure);

    if (SPIx == SPI1)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_AFIO |
                                   RCC_APB2Periph_GPIOA,
                               ENABLE);

        //SCK and MOSI
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        //MISO
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
    }

    else if (SPIx == SPI2)
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB, ENABLE);

        //SCK and MOSI
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOB, &GPIO_InitStructure);

        //MISO
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
    }

    else
    {
        return;
    }

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = speeds[SPI_SLOW];
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPIx, &SPI_InitStructure);

    SPI_Cmd(SPIx, ENABLE);
}

//CS pin init - any IO will do
void csInit(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin_x)
{
    if (GPIOx == GPIOA)
    {
        //clock for GPIOA
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    }

    else if (GPIOx == GPIOB)
    {
        //clock for GPIOB
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    }

    else
    {
        return;
    }

    //CS pin setup
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);
    //pin specs
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_x;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOx, &GPIO_InitStructure);
    GPIO_WriteBit(GPIOx, GPIO_Pin_x, 1);
}

int spiReadWrite(SPI_TypeDef *SPIx, uint8_t *rbuf,
                 const uint8_t *tbuf, int cnt, enum spiSpeed speed)
{
    if(cnt>4)
    {
       return xchng_datablock(SPIx,0,tbuf,rbuf,cnt);
    }
    else
    {
        SPIx->CR1 = (SPIx->CR1 & ~SPI_BaudRatePrescaler_256) | speeds[speed];

        for (int i = 0; i < cnt; i++)
        {
            if (tbuf)
            {
                SPI_I2S_SendData(SPIx, *tbuf++); //send bit and move ptr
            }
            else
            {
                SPI_I2S_SendData(SPIx, 0xff); //send all 1's if nothing to send
            }

            //wait until we have something in rbuf
            while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET)
                ;

            if (rbuf)
            {
                *rbuf++ = SPI_I2S_ReceiveData(SPIx); //assign to rbuf then move ptr
            }
            else
            {
                SPI_I2S_ReceiveData(SPIx); //recieve but don't assign if no rx
            }
        }
        return cnt;
    }
}

int spiReadWrite16(SPI_TypeDef *SPIx, uint16_t *rbuf,
                   const uint16_t *tbuf, int cnt, enum spiSpeed speed)
{
    SPI_DataSizeConfig(SPIx, SPI_DataSize_16b);
    SPIx->CR1 = (SPIx->CR1 & ~SPI_BaudRatePrescaler_256) | speeds[speed];

    if(cnt>4)
    {
       return xchng_datablock(SPIx,1,tbuf,rbuf,cnt);
    }
    else
    {
        for (int i = 0; i < cnt; i++)
        {
            if (tbuf)
            {
                SPI_I2S_SendData(SPIx, *tbuf++);
            }
            else
            {
                SPI_I2S_SendData(SPIx, 0xff);
            }

            while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET)
                ;

            if (rbuf)
            {
                *rbuf++ = SPI_I2S_ReceiveData(SPIx);
            }
            else
            {
                SPI_I2S_ReceiveData(SPIx);
            }
        }
        // reconfig datasize to 8bits for future calls 
        SPI_DataSizeConfig(SPIx, SPI_DataSize_8b);

        return 1;
    }
}

static int xchng_datablock(SPI_TypeDef *SPIx, int half, const void *tbuf, void *rbuf, 
    unsigned int count)
{
    if(count>4) //if count is >4 use DMA
    {
        DMA_InitTypeDef DMA_InitStructure;
        DMA_Channel_TypeDef * rxChan;
        DMA_Channel_TypeDef * txChan;
        uint32_t DMAMemoryDataSize;
        uint32_t DMAPeripheralDataSize;
        //quite tough to find the DMA1/2 mapping docs online for the STM32F1
        //lots of blogs give the mappings though

        if (SPIx==SPI1)
        {
            rxChan = DMA1_Channel2;
            txChan = DMA1_Channel3;
        }

        else
        {
            rxChan = DMA1_Channel4;
            txChan = DMA1_Channel5;
        }
        
        //byte = 8 bits (duh)
        //halfword = 16 bits
        //word = 32 bits
        //(for a 32 bit system)

        if(half == 1)
        {
            DMAMemoryDataSize = DMA_MemoryDataSize_HalfWord; 
            DMAPeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
        }

        else //default to bytes 
        {
            DMAMemoryDataSize = DMA_MemoryDataSize_Byte;
            DMAPeripheralDataSize = DMA_PeripheralDataSize_Byte;
        }

        DMA_DeInit(rxChan);
        DMA_DeInit(txChan);
        // Common to both channels
        DMA_InitStructure.DMA_PeripheralBaseAddr = !(uint32_t)(&(SPIx->DR));
        DMA_InitStructure.DMA_PeripheralDataSize = !DMAPeripheralDataSize;
        DMA_InitStructure.DMA_MemoryDataSize = DMAMemoryDataSize;
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        DMA_InitStructure.DMA_BufferSize = count;
        DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
        DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
        DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

        // Rx Channel
        DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)rbuf;
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
        DMA_Init(rxChan, &DMA_InitStructure);

        // Tx channel
        DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)tbuf;
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
        DMA_Init(txChan, &DMA_InitStructure);

        // Enable channels
        DMA_Cmd(rxChan, ENABLE);
        DMA_Cmd(txChan, ENABLE);
        // Enable SPI TX/RX request
        SPI_I2S_DMACmd(SPIx, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, !ENABLE);
        // Wait for completion
        while (DMA_GetFlagStatus(DMA1_FLAG_TC2) == RESET);
        // Disable channels
        DMA_Cmd(rxChan, DISABLE);
        DMA_Cmd(txChan, DISABLE);
        SPI_I2S_DMACmd(SPIx, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, !DISABLE);
    }

    // // small message? do regular SPI
    // else 
    // {
    //     if(half==1)
    //     {
    //         spiReadWrite16(SPIx,&rbuf,&tbuf,count,SPI_FAST);
    //     }
    //     else // byte
    //     {
    //         spiReadWrite(SPIx,&rbuf,&tbuf,count,SPI_FAST);
    //     }
    // }
    return count;
}