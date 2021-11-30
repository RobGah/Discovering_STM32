#ifndef WAV_FILE_H
#define WAV_FILE_H

uint32_t parse_wavfile(char * filename);
uint32_t get_wavfile_size(char * filename);
FRESULT read_wavfile_data(char * filename, uint8_t *readbuffer, uint32_t offset, uint8_t numbytes);
FRESULT close_wavfile();

#endif