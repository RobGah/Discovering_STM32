#ifndef EEPROM_H
#define EEPROM_H

#define CS_PIN               GPIO_Pin_12
#define CS_PORT              GPIOB
#define SPI_USED             SPI2      
#define WIP(x)               ((x)&1)  
#define PAGE_SIZE            16
#define LAST_ADDRESS         0x07FF
#define MAX_WRITE_BYTES      16


enum eepromCMD
{
    cmdREAD = 0x03, 
    cmdWRITE = 0x02,
    cmdWREN = 0x06,
    cmdWRDI = 0x04,
    cmdRDSR = 0x05,
    cmdWRSR = 0x01
};


void eepromInit();

void eepromWriteEnable();

void eepromWriteDisable();

uint8_t eepromReadStatus();

void eepromWriteStatus(uint8_t status);

int eepromWrite(uint8_t *buf, uint8_t cnt, uint16_t address);

int eepromRead(uint8_t *buf, uint8_t cnt, uint16_t address);


#endif

