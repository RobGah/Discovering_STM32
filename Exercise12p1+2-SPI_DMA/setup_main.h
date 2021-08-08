#ifndef SETUP_MAIN_H
#define SETUP_MAIN_H

#define LED_PORT  GPIOC
#define LED_PIN  GPIO_Pin_13

//LED
void init_onboard_led(void);

//timer stuff
void Delay(uint32_t nTime);
void SysTick_Handler(void);

//generic GPIO setup
void init_GPIO_pin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pinx, int GPIO_Mode, int GPIO_Speed);

#endif