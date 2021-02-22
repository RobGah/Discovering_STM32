#ifndef NUNCHUK_H
#define NUNCHUK_H

void nunchuk_init(I2C_TypeDef *I2C,int I2C_Speed, uint8_t address);
void report_nunchuk_data(I2C_TypeDef * I2C, uint8_t I2C_address);

#endif