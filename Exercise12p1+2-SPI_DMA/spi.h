#ifndef SPI_H
#define SPI_H

enum spiSpeed
{
    SPI_SLOW,
    SPI_MEDIUM,
    SPI_FAST
};

static const uint16_t speeds[] =
{
    [SPI_SLOW] = SPI_BaudRatePrescaler_64,
    [SPI_MEDIUM] =  SPI_BaudRatePrescaler_8,
    [SPI_FAST] = SPI_BaudRatePrescaler_2
};

void spiInit(SPI_TypeDef *SPIx);
//Initializes SPI comms when given SPI device

int spiReadWrite(SPI_TypeDef *SPIx, uint8_t *rbuf, 
    const uint8_t *tbuf, int cnt, enum spiSpeed speed);
/*
Sends and recieves arrays that are cnt long of 8-bit words at a chosen speed 1, 2 or 3 
(SPI_SLOW,SPI_MEDIUM,SPI_FAST)
*/ 

int spiReadWrite16(SPI_TypeDef *SPIx, uint16_t *rbuf, 
    const uint16_t *tbuf, int cnt, enum spiSpeed speed);
/*
Sends and recieves arrays that are cnt long of 16-bit words at a chosen speed 1, 2 or 3 
(SPI_SLOW,SPI_MEDIUM,SPI_FAST)
*/

void csInit(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin_x);

int xchng_datablock(SPI_TypeDef *SPIx, int half, const void *tbuf, void *rbuf, unsigned count);


#endif