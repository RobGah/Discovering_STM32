#ifndef NUNCHUKSERVO_H
#define NUNCHUKSERVO_H

void write_joystick_to_servo(I2C_TypeDef *I2Cx,uint8_t I2C_address,uint8_t *data);

void init_nunchuk_servo_peripherals(I2C_TypeDef *I2Cx, int I2CClockSpeed, 
    uint8_t I2C_address);


#endif