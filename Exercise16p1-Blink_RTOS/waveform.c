#include <math.h>
#include <stdint.h>
#include "dac.h"

//easy single file gcc compile : "gcc -g -Wall -owaveform waveform.c"
// and run w/ "./waveform.exe"

void gen_sine_wave(uint16_t * wavetable, uint16_t num_samples,uint16_t min_amp, uint16_t max_amp)
{
    /*
    inputs:
    -wavetable: the array (pointer) to where we want to write the sine wave data
    -num_samples: the number of samples to generate
    -min_amp: the minimum amplitude of sine data that we want
    -max_amp: the max amplitude of sine data that we want

    outputs:
    -nothing (we write the data to wavetable) 

    */
    

    float sin_wave[num_samples]; // ew gross floating point
    
    // Step 1: generate sine wave table
    for(uint16_t i = 0; i<num_samples; i++)
    {
        // wave[i] = sin(2*pi*freq*t)
        // assume freq=1? 
        sin_wave[i]= sin(2*acos(-1)*(float)(i/(float)num_samples));
    }

    // Step 2: map samples from sine wave table to min and max
    for(uint16_t i = 0; i<num_samples;i++)
    {
        //output = output_start + ((output_end - output_start) / (input_end - input_start)) * (input - input_start)
        wavetable[i] = min_amp + ((max_amp - min_amp) / (2)) * (sin_wave[i] - (-1));
    }

}

#define USE_APP

#ifndef USE_APP

uint16_t wave[100];
uint16_t num_samples = 100;
uint16_t min_amp = 512;
uint16_t max_amp = 1536;


int main()
{
    gen_sine_wave(wave,num_samples,min_amp,max_amp);
    for(int i=0; i<num_samples;i++)
    {
        printf("%d\r\n",wave[i]);
    }
    return 0;
}
#endif