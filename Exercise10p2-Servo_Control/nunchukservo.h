#ifndef NUNCHUKSERVO_H
#define NUNCHUKSERVO_H

void write_joystick_to_servo(void);

void init_nunchuk_servo_peripherals(I2C_TypeDef *I2Cx, int I2CClockSpeed, 
    uint8_t I2C_address, GPIO_TypeDef * TIM2GPIOx, GPIO_TypeDef *TIM1GPIOx, 
    uint16_t TIM2_pin, uint16_t TIM1_pin);


#endif