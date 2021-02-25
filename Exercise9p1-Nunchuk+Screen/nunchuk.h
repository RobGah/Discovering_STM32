#ifndef NUNCHUK_H
#define NUNCHUK_H

void nunchuk_init(uint8_t I2C_address);
void report_nunchuk_data(I2C_TypeDef * I2C, uint8_t I2C_address);

#endif