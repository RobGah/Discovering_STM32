#ifndef SPI_H
#define SPI_H

enum spiSpeed
{
    SPI_SLOW,
    SPI_MEDIUM,
    SPI_FAST
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
#endif