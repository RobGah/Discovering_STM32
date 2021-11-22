#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#define AUDIOBUFSIZE 128

extern uint8_t Audiobuf [];
extern bool audioplayerHalf;
extern bool audioplayerWhole;

void audioplayerInit();
void audioplayerStart(char * file);
void audioplayerStop(char * file);
FRESULT audioplayerLoadFile(char *file, char* readbuffer,uint32_t datasize, uint32_t offset);
FRESULT audioplayerNextBlock(char *file, char *readbuffer, uint32_t offset, uint16_t numbytes);


#endif