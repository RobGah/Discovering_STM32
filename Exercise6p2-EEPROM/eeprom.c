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

    //uint8_t cmd = cmdWRSR; // spi demands a ptr

    //cmd[0] = command byte and cmd[1] = status byte
    uint8_t cmd[2] = {cmdWRSR, status};
    //uint8_t res[2]; //throwaways from slave?

    while (WIP(eepromReadStatus())); //wait until writing is done!
   
    //unsure if I have to issue write enable to write status?
    GPIO_WriteBit(CS_PORT, CS_PIN, 0); 
    spiReadWrite(SPI_USED, 0, cmd, 2, SPI_SLOW); //null pointer to rbuf seems to be ok?
    GPIO_WriteBit(CS_PORT, CS_PIN, 1);
   
}

int eepromRead(uint8_t *buf, uint8_t cnt, uint16_t address) 
{
    /* 
    reads cnt bytes data beginning at address
    
    What it does:
    -memory wrap read if a read operation exceeds LAST_ADDRESS
    -does not read if address requested is greater than LAST_ADDRESS
    -a read w/ no wrap returns 0
    -the user ought to know what the last memory address is anyway.       
    */

    uint8_t cmd = cmdREAD;
    uint16_t last_address_requested = address + cnt;
    uint16_t flipped_address = flip_address(address);

    if(address > LAST_ADDRESS)
    {
        return(-(address - LAST_ADDRESS)); //negative means we overshot the call by that much
        //do not write to an out-of-bounds address
    }

    while (WIP(eepromReadStatus())); //wait until writing is done!

    GPIO_WriteBit(CS_PORT,CS_PIN, 0);
    spiReadWrite(SPI_USED, 0, &cmd, 1, SPI_SLOW); //send read cmd - wants a ptr hence the cmd
    spiReadWrite16(SPI_USED, 0, &flipped_address, 1, SPI_SLOW); //send address to start read from
    spiReadWrite(SPI_USED, buf, 0, cnt, SPI_SLOW); //read cnt bytes, store bytes at buf
    GPIO_WriteBit(CS_PORT,CS_PIN, 1);

    if(last_address_requested > LAST_ADDRESS)
    {
        return((last_address_requested - LAST_ADDRESS)); //positive integer indicates a wrap by that much
    }
    else
    {
        return(0); //read with 0 wrap
    }
    
}

int eepromWrite(uint8_t *buf, uint8_t cnt, uint16_t address)
{
    /* writes bytes to memory 

    Accomplishes the following:
    -Handles EEPROM writing and memory address management
    -Tracks page usage and generates a new page when needed
    -Tracks memory usage and does not write to memory if we specify a number of bytes that exceeds the last memory byte
    */

    //initial variables
    int bytes_left_in_page = PAGE_SIZE - (address % PAGE_SIZE); //bytes remaining in page given initial address
    uint16_t current_address = address; //track addresss we're on
    uint16_t last_address_requested = current_address + cnt;
    uint16_t flipped_address = flip_address(current_address); //address to pass to 16 byte SPI call w/ LSByte in front
    uint8_t cmd = cmdWRITE;

    //get count of new pages needed based on size of cnt passed in
    int new_pages_needed = 0;
    int byte_cnt_for_pages, cnt_left_to_write = cnt; //one for determining pages needed, other for determing cnt to write to eeprom each write cycle
    int initial_page_bytes = bytes_left_in_page; //bytes_left_in_page is useful, no need to alter it. 

    if(last_address_requested > LAST_ADDRESS)
    {
        return (-(last_address_requested - LAST_ADDRESS)); //returning a negative # means we overshot the last address on the call by that much
    }
    
    if(byte_cnt_for_pages > initial_page_bytes) //if there's more to write than what we can fit on the initial page
    {
        while(byte_cnt_for_pages > 0) //while we still have bytes to be page'd
        {
            ++new_pages_needed; //increment new pages needed
            byte_cnt_for_pages = byte_cnt_for_pages - initial_page_bytes; //subtract bytes left in page from total bytes
            initial_page_bytes = PAGE_SIZE; //reset initial_page_bytes to max bytes per page
        }
    }

    /*
    We have defined :
    -the initial address passed in
    -the number of bytes requested to be written
    -the bytes left in the page given the initial address
    -a variable to track the current address
    -a count of new pages needed after initial address
    -the last address of the eeprom and a way to prevent an overshoot
    */

    while(WIP(eepromReadStatus())); // do nothing while write is going on.

    //initial write
    eepromWriteEnable(); 
    GPIO_WriteBit(CS_PORT, CS_PIN, 0);
    spiReadWrite(SPI_USED, 0, &cmd, 1, SPI_SLOW);
    spiReadWrite16(SPI_USED, 0, &flipped_address, 1, SPI_SLOW);
    spiReadWrite(SPI_USED, 0, buf, bytes_left_in_page, SPI_SLOW);
    GPIO_WriteBit(CS_PORT,CS_PIN, 1);

    //increment address and decrement cnt
    current_address = current_address + bytes_left_in_page;
    flipped_address = flip_address(current_address);
    cnt_left_to_write = cnt_left_to_write - bytes_left_in_page;

    //loop to create pages
    for(new_pages_needed; new_pages_needed>0; new_pages_needed--)
    {    
        
        if(cnt_left_to_write > MAX_WRITE_BYTES)
        {
            while(WIP(eepromReadStatus())); // do nothing while a write is going on.

            eepromWriteEnable(); 
            GPIO_WriteBit(CS_PORT, CS_PIN, 0);
            spiReadWrite(SPI_USED, 0, &cmd, 1, SPI_SLOW);
            spiReadWrite16(SPI_USED, 0, &flipped_address, MAX_WRITE_BYTES, SPI_SLOW);
            spiReadWrite(SPI_USED, 0, buf, MAX_WRITE_BYTES, SPI_SLOW);
            GPIO_WriteBit(CS_PORT,CS_PIN, 1);

            //increment address and decrement cnt
            current_address = current_address + MAX_WRITE_BYTES;
            flipped_address = flip_address(current_address);
            cnt_left_to_write = cnt_left_to_write - MAX_WRITE_BYTES;
        }
        else
        {
            while (WIP(eepromReadStatus())); //wait until writing is done!

            eepromWriteEnable(); 
            GPIO_WriteBit(CS_PORT, CS_PIN, 0);
            spiReadWrite(SPI_USED, 0, &cmd, 1, SPI_SLOW);
            spiReadWrite16(SPI_USED, 0, &flipped_address, MAX_WRITE_BYTES, SPI_SLOW);
            spiReadWrite(SPI_USED, 0, buf, cnt_left_to_write, SPI_SLOW);
            GPIO_WriteBit(CS_PORT,CS_PIN, 1);

            //increment address and decrement cnt
            current_address = current_address + cnt_left_to_write;
            flipped_address = flip_address(current_address);
            cnt_left_to_write = cnt_left_to_write - cnt_left_to_write;
        } 
   }
   return(current_address); //a positive number
}

uint16_t flip_address(uint16_t val)
{
    /* 
    SPI sends in LSByte first for 16 bit mode
    Nessessary to swap MSByte w/ LSByte

    Cute trick I found online
    */

    return((val>>8) | (val<<8));
}

