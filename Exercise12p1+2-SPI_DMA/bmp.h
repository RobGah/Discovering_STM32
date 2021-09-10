#include <stdint.h>
#include "ff.h"

#ifndef BMP_H
#define BMP_H

//from author
struct bmpfile_magic 
{
    // refers to magic numbers aka file type signature
    // 2 bytes or 4 hex values
    unsigned char magic [2]; 
};

struct bmpfile_header 
{
    uint32_t filesz;
    uint16_t creator1;
    uint16_t creator2;
    uint32_t bmp_offset;
};

typedef struct 
{
    uint32_t header_sz;
    int32_t width;
    int32_t height;
    uint16_t nplanes;
    uint16_t bitspp;
    uint32_t compress_type;
    uint32_t bmp_bytesz;
    int32_t hres;
    int32_t vres;
    uint32_t ncolors;
    uint32_t nimpcolors;
} BITMAPINFOHEADER;

struct bmppixel 
{ // little endian byte order
    uint8_t b;
    uint8_t g;
    uint8_t r;
    //uint8_t dummy;
};

struct bmpfile_magic magic;
struct bmpfile_header header;
BITMAPINFOHEADER info;
struct bmppixel pixel;

#endif


