#ifndef DAC_H
#define DAC_H

int DAC_init(uint32_t channel);
int DAC_init_w_Trig(uint32_t channel, uint32_t trigger_src);
int init_dac_dma (void *outbuf, unsigned int count);
void gen_sine_wave(uint16_t * wavetable, uint16_t num_samples,uint16_t min_amp, uint16_t max_amp);


#endif