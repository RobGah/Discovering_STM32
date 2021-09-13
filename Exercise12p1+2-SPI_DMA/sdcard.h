#ifndef SDCARD_H
#define SDCARD_H

// GLOBAL SD Card Variables
FATFS FatFs;		/* FatFs work area needed for each volume */
FIL file;			/* File object needed for each open file */
UINT bmp_count; /* ad oculos*/
FRESULT fr;         /* FatFs function common result code*/
DIR dir;
uint8_t first_file_parsed_flag;

FRESULT scan_files(char* path);
UINT scan_bmp_files (char* path);
FRESULT parse_BMP_file(char* path);
FRESULT get_BMP_image(char* path);


#endif