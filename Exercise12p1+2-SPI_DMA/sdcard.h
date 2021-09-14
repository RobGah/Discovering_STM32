#ifndef SDCARD_H
#define SDCARD_H

// GLOBAL SD Card Variables
// This works but this technically is bad form, I ought to use extern to declare here
// and define my variables in sdcard.c and main.c, but it works.
FATFS FatFs;		/* FatFs work area needed for each volume */
FIL file;			/* File object needed for each open file */
UINT bmp_count;     /* ad oculos*/
FRESULT fr;         /* FatFs function common result code*/
DIR dir;
uint8_t first_file_parsed_flag;
static FILINFO fno;

FRESULT scan_files(char* path);
UINT scan_bmp_files (char* path);
FRESULT parse_BMP_file(char* path);
FRESULT get_BMP_image(char* path);
FRESULT get_BMP_image_DMA(char* path, uint16_t *bit_buffer);


#endif