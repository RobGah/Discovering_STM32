#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_spi.h>
#include "spi.h"
#include "eeprom.h"

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

void eepromInit()
{
    /*
    -initializes all GPIO and settings for SPI2 
    -initializes CS pin with CS init for chosen CS pin
    */
    csInit(CS_PORT, CS_PIN);

    spiInit(SPI_USED);
}

uint8_t eepromReadStatus()
{
    /* 
    -Checks read status of eeprom IC - returns 1 if ok to write or 0 if not
    */

   //cmd[0] = command byte and cmd[1] = empty byte while listening
   uint8_t cmd[] = {cmdRDSR, 0xff}; 
   //res[0] is throwaway byte while writing command, res[1] byte has status
   uint8_t res[2]; 
   GPIO_WriteBit(CS_PORT, CS_PIN, 0); 
   spiReadWrite(SPI_USED, res, cmd, 2, SPI_SLOW);
   GPIO_WriteBit(CS_PORT, CS_PIN, 1);
   return res[1]; //return res element w/ status
}

void eepromWriteEnable()
{
    /*
    -Enables writing to eeprom
    */
   uint8_t cmd = cmdWREN;

   while (WIP(eepromReadStatus())); //while read status is 1&1 aka write is in progress

   GPIO_WriteBit(CS_PORT, CS_PIN, 0);
   spiReadWrite(SPI_USED, 0, &cmd, 1, SPI_SLOW); //read nothing, send command to enable
   GPIO_WriteBit(CS_PORT, CS_PIN, 1);
}

void eepromWriteDisable()
{
    /*
    -Disables writing to eeprom
    */
   
   uint8_t cmd = cmdWRDI;

   while (WIP(eepromReadStatus())); //while read status is 1&1 aka write is in progress

   GPIO_WriteBit(CS_PORT, CS_PIN, 0);
   spiReadWrite(SPI_USED, 0, &cmd, 1, SPI_SLOW); //read nothing, send command to disable
   GPIO_WriteBit(CS_PORT, CS_PIN, 1);
}

void eepromWriteStatus(uint8_t status)
{
    // Writes 8 bit status to eeprom status register
    //status write is for BP1(bit 3) and BP0(bit 2) - Bits 7-4 and 1-0 are read only

    //cmd[0] = command byte and cmd[1] = status byte
    uint8_t cmd[2] = {cmdWRSR, status};

    while (WIP(eepromReadStatus())); //wait until writing is done!
   
    GPIO_WriteBit(CS_PORT, CS_PIN, 0); 
    spiReadWrite(SPI_USED, 0, cmd, 2, SPI_SLOW);
    GPIO_WriteBit(CS_PORT, CS_PIN, 1);
   
}

int eepromRead(uint8_t *buf, uint8_t cnt, uint16_t address) 
{
    /* 
    reads cnt bytes of data beginning at address
    the user ought to know what the last memory address is or else a wrap occurs.       
    */

    uint8_t cmd = cmdREAD;
    while (WIP(eepromReadStatus())); //wait until writing is done!

    GPIO_WriteBit(CS_PORT,CS_PIN, 0);
    spiReadWrite(SPI_USED, 0, &cmd, 1, SPI_SLOW); //send read cmd - wants a ptr hence the cmd
    spiReadWrite16(SPI_USED, 0, &address, 1, SPI_SLOW); //send address to start read from
    spiReadWrite(SPI_USED, buf, 0, cnt, SPI_SLOW); //read cnt bytes, store bytes at buf
    GPIO_WriteBit(CS_PORT,CS_PIN, 1);

    return(0);
    
}

int eepromWrite(uint8_t *buf, uint8_t cnt, uint16_t address)
{
    /* 
    -Handles EEPROM writing and memory address management
    -Tracks page usage and generates a new page when needed
    */

    //initial variables / calcs
    int initial_page_bytes = PAGE_SIZE - (address % PAGE_SIZE); //bytes remaining in page given initial address
    uint16_t current_address = address; //track addresss we're on
    uint8_t cmd = cmdWRITE;

    while(WIP(eepromReadStatus())); // do nothing while write is going on.

    //initial write
    eepromWriteEnable(); 
    GPIO_WriteBit(CS_PORT, CS_PIN, 0);
    spiReadWrite(SPI_USED, 0, &cmd, 1, SPI_SLOW);
    spiReadWrite16(SPI_USED, 0, &current_address, 1, SPI_SLOW);
    spiReadWrite(SPI_USED, 0, buf, initial_page_bytes, SPI_SLOW);
    GPIO_WriteBit(CS_PORT,CS_PIN, 1);

    //increment address and buf ptr and decrement cnt
    current_address = current_address + initial_page_bytes;
    cnt = cnt - initial_page_bytes;
    buf = buf + initial_page_bytes;

    //loop to create pages
    while(cnt >0)    
    {    
        
        if(cnt > MAX_WRITE_BYTES)
        {
            while(WIP(eepromReadStatus())); // do nothing while a write is going on.

            eepromWriteEnable(); 
            GPIO_WriteBit(CS_PORT, CS_PIN, 0);
            spiReadWrite(SPI_USED, 0, &cmd, 1, SPI_SLOW);
            spiReadWrite16(SPI_USED, 0, &current_address, 1, SPI_SLOW);
            spiReadWrite(SPI_USED, 0, buf, MAX_WRITE_BYTES, SPI_SLOW);
            GPIO_WriteBit(CS_PORT,CS_PIN, 1);

            //increment address and buf ptr and decrement cnt
            current_address = current_address + MAX_WRITE_BYTES;
            cnt = cnt - MAX_WRITE_BYTES;
            buf = buf + MAX_WRITE_BYTES;

        }
        else
        {
            while (WIP(eepromReadStatus())); //do nothing while a write is going on.


            eepromWriteEnable(); 
            GPIO_WriteBit(CS_PORT, CS_PIN, 0);
            spiReadWrite(SPI_USED, 0, &cmd, 1, SPI_SLOW);
            spiReadWrite16(SPI_USED, 0, &current_address, 1, SPI_SLOW);
            spiReadWrite(SPI_USED, 0, buf, cnt, SPI_SLOW);
            GPIO_WriteBit(CS_PORT,CS_PIN, 1);

            //increment address and buf ptr and decrement cnt
            current_address = current_address + cnt;
            cnt = 0; //end loop
            buf = buf + MAX_WRITE_BYTES;

        } 
   }
   return(current_address); //a positive number
}


