#ifndef EEPROM_H
#define EEPROM_H

void eepromInit();

void eepromWriteEnable();

void eepromWriteDisable();

uint8_t eepromReadStatus();

void eepromWriteStatus(uint8_t status);

int eepromWrite(uint8_t *buf, uint8_t cnt, uint16_t address);

int eepromRead(uint8_t *buf, uint8_t cnt, uint16_t address);


#endif

