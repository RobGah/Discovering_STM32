#ifndef NUNCHUK_H
#define NUNCHUK_H

//basic defines live here
#define ACCEL_BITS 1023
#define MILLIG_DYNAMIC_RANGE 4000

//nunchuk variables
uint8_t joystick_x;
uint8_t joystick_y;
uint16_t accel_x_raw;
uint16_t accel_y_raw;
uint16_t accel_z_raw;
bool c_button;
bool z_button;

void nunchuk_init(I2C_TypeDef *I2C, int I2CClockSpeed, uint8_t I2C_address);
void update_nunchuck_data(I2C_TypeDef *I2C, uint8_t I2C_address,uint8_t *data);
void report_nunchuk_data(I2C_TypeDef * I2C, uint8_t I2C_address,uint8_t *data);

#endif