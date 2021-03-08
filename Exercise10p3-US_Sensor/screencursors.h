#ifndef SCREENCURSORS_H
#define SCREENCURSORS_H

/*
Module enables Wii Nunchuk to control cursors on ST7735R screen.
*/

void init_screencursor_peripherals(I2C_TypeDef *I2C, int I2CClockSpeed, uint8_t I2C_address);
void update_cursors_on_screen(I2C_TypeDef *I2C, uint8_t I2C_address,uint8_t *data);


#endif