#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#define AUDIOBUFSIZE 128

#include <stdbool.h>

extern uint8_t Audiobuf [];
extern bool audioplayerHalf;
extern bool audioplayerWhole;

void audioplayerInit();
void audioplayerStart();
void audioplayerStop();
FRESULT audioplayerLoadFile(char *file, char* readbuffer,uint32_t datasize, uint32_t offset);
FRESULT audioplayerNextBlock(char *file, char *readbuffer, uint32_t offset, uint16_t numbytes);


#endif