
#include <stdint.h>
#include <stdbool.h>
#include <stm32f10x_tim.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_dac.h>
#include "timers.h"
#include "ff.h"
#include "interrupts.h"
#include "wav_file.h"
#include "audioplayer.h"


void audioplayerInit() // author passes uint samplerate, but its not needed?
{
    // Timer Init
    // This is 'close enough' - did both math (see worksheet)
    // and verified with mikro calculator program for timers
    // DANGER - prescalar/divided clock must be an integer value
    timer_init(TIM3, RCC_APB1Periph_TIM3, 4000000, 
        91, TIM_CounterMode_Up); 

    // Config output to trigger on an update
    TIM_SelectOutputTrigger(TIM3 , TIM_TRGOSource_Update);
    TIM_ITConfig(TIM3 , TIM_IT_Update , ENABLE);
    
    // Enable Interrupt
    config_NVIC(DMA1_Channel3_IRQn,0);

    // INIT DAC
    DAC_init_w_Trig(DAC_Channel_1,DAC_Trigger_T3_TRGO);
    init_dac_dma(Audiobuf,AUDIOBUFSIZE);
}

FRESULT audioplayerLoadFile(char *file, char* readbuffer, uint32_t datasize, uint32_t offset)
{
    /* loads file, gets buffer FULLY loaded w/ data before calling play
    
    Takes a file, a pointer to the readbuffer and, 
    
    
    */

    FRESULT fr;

    uint32_t filesize = get_wavefile_size(&file); // total size of file
    offset = parse_wavfile(&file); // offset to song data
    datasize = filesize - offset; // total size of data chunk
    /* 
    initial read so that read buffer has something when we go to play
    big note - read_wavefil_data INCREMENTS offset for us. 
    */
    fr = read_wavfile_data(&file,&readbuffer,offset,AUDIOBUFSIZE);

    return fr;
}

FRESULT audioplayerNextBlock(char *file, char *readbuffer, uint32_t offset, uint16_t numbytes)
{
    // wrapper for read_wavefile_data
    // Should this be in an interface?
    // I dunno if this is really that great philosophically but I'm rolling w it
    FRESULT fr = read_wavfile_data(&file,&readbuffer,offset,numbytes);
    return fr;
}
void audioplayerStart(char *file)
{
    //enable timer - this sets everything in motion i.e. the IRQHandler
    TIM_Cmd(TIM3 , ENABLE);
    // Enable DAC
    DAC_Cmd(DAC_Channel_1 , ENABLE);
    DMA_Cmd(DMA1_Channel3 , ENABLE);
    DAC_DMACmd(DAC_Channel_1 , ENABLE);

}
void audioplayerStop(char *file)
{
   //enable timer - this sets everything in motion i.e. the IRQHandler
    TIM_Cmd(TIM3 , DISABLE);
    // Enable DAC
    DAC_Cmd(DAC_Channel_1 , DISABLE);
    DMA_Cmd(DMA1_Channel3 , DISABLE);
    DAC_DMACmd(DAC_Channel_1 , DISABLE);
    // close audio file
    f_close(&file);  
}
