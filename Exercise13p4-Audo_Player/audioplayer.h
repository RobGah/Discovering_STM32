#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#define AUDIOBUFSIZE 128

extern uint8_t Audiobuf [];
extern bool audioplayerHalf;
extern bool audioplayerWhole;

audioplayerInit(unsigned int sampleRate);
audioplayerStart();
audioplayerStop();

#endif