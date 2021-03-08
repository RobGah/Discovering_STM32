/* Module to control hobby microservos w/ wii nunchuk*/

#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_tim.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "uart.h"
#include "spi.h"
#include "timers.h"
#include "nunchuk.h"
#include "setup_main.h"
#include "xprintf.h"
#include "i2c.h"
#include "nunchukservo.h"
#include "setup_main.h"

#define SCALE_JOY_SERVO (255/0.01) //255 steps over .01ms pulse (assuming 100hz timer signal)


int scaled_servo_position[2];


static void convert_nunchuk_input_to_servo_pw(void)
{
    /*Scale and offset operation on the joystick to convert joystick inputs to servo pulses. */

    scaled_servo_position[0] = (int)((joystick_x/SCALE_JOY_SERVO + .01)*10000);
    scaled_servo_position[1] = (int)((joystick_y/SCALE_JOY_SERVO + .01)*10000);
}

void init_nunchuk_servo_peripherals(I2C_TypeDef *I2Cx, int I2CClockSpeed, 
    uint8_t I2C_address)
{
    nunchuk_init(I2Cx, I2CClockSpeed,I2C_address);
    xprintf("Nunchuk Initialized\r\n");
    pwm_init(TIM4,RCC_APB1Periph_TIM4,100000,1000,TIM_CounterMode_Up,3);
    xprintf("TIM4 CH3 initialized\r\n");
    pwm_init(TIM4,RCC_APB1Periph_TIM4,100000,1000,TIM_CounterMode_Up,4);
    xprintf("TIM4 CH4 initialized\r\n");
    init_GPIO_pin(GPIOB, GPIO_Pin_8, GPIO_Mode_AF_PP, GPIO_Speed_50MHz);
    xprintf("GPIOB Pin 8 initialized for TIM4 (Channel 3)\r\n");
    init_GPIO_pin(GPIOB, GPIO_Pin_9, GPIO_Mode_AF_PP, GPIO_Speed_50MHz);
    xprintf("GPIOB Pin 9 initialized for TIM4 (Channel 4)\r\n");


}

void write_joystick_to_servo(I2C_TypeDef *I2Cx,uint8_t I2C_address,uint8_t *data)
{
    update_nunchuck_data(I2Cx,I2C_address,data);
    convert_nunchuk_input_to_servo_pw();
    TIM_SetCompare3(TIM4,scaled_servo_position[0]);
    TIM_SetCompare4(TIM4,scaled_servo_position[1]);
}