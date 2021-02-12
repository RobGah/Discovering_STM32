#ifndef MISC_H
#define MISC_H

#define LED_PORT  GPIOC
#define LED_PIN  GPIO_Pin_13

//LED
void init_onboard_led(void);

//timer stuff
void Delay(uint32_t nTime);
void SysTick_Handler(void);
#endif