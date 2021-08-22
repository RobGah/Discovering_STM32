#ifndef SDCARD_H
#define SDCARD_H

FRESULT scan_files(char* path);
UINT scan_bmp_files (char* path);
FRESULT parse_BMP_file(char* path);


#endif