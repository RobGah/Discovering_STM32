#ifndef TIMERS_H
#define TIMERS_H

void pwm_init(TIM_TypeDef * TIMx, uint32_t timerperiph, uint32_t prescaler_div,
    uint32_t period,uint16_t countermode, int channel);

void init_input_pw_capture(TIM_TypeDef * TIMx, uint32_t timerperiph, uint32_t prescaler_div,
    uint32_t period,uint16_t countermode, int channel,uint16_t polarity, uint16_t selection,
    uint16_t input_trigger, bool init_complete);

#endif