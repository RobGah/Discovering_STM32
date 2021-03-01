#ifndef TIMERS_H
#define TIMERS_H

void timer_init(TIM_TypeDef * TIMx, uint32_t timerperiph, uint32_t prescaler_div, 
    uint32_t period,uint16_t countermode);

#endif