#include <stdint.h>
#include <string.h>
#include "ff.h"
#include "xprintf.h"
#include "sdcard.h"
#include <stdbool.h>
#include "bmp.h"
#include "LCD7735R.h"


FRESULT scan_files(char* path)
{
    // if you want the default directory just pass in "" for path 
    FRESULT res;
    DIR dir;
    UINT i;

    res = f_opendir(&dir, path);
    xprintf("f_opendir() returns %d\r\n",res);
    
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno); // Read a directory item 
            xprintf("f_readdir() returns %d\r\n",res);
            if (res != FR_OK || fno.fname[0] == 0) 
            {
                xprintf("breaking 1!\r\n");
                break;  // Break on error or end of dir 
            }
            if (fno.fattrib & AM_DIR)  // It is a directory 
            {                   
                i = strlen(path);
                xsprintf(&path[i], "/%s", fno.fname); // xprintf.h is a hero 
                res = scan_files(path);                    // Enter the directory 
                if (res != FR_OK) 
                {
                    xprintf("breaking 2!\r\n");
                    break;
                }
                path[i] = 0;
            } 
            else 
            {                                       // It is a file.
                xprintf("%s/%s\n", path, fno.fname);
            }
        }
        f_closedir(&dir);
    }

    return res;
} 

UINT scan_bmp_files(char* path)
{
    UINT i = 0;     // Retval for count of BMPs

    fr = f_findfirst(&dir, &fno, "", "*.bmp"); // Start to search for photo files 
    xprintf("f_findfirst() returns %d\r\n",fr);
    if (fr != FR_OK)
    {
        return i;
    }

    while (fr == FR_OK && fno.fname[0]) 
    {                                             // Repeat while an item is found
        xprintf("%s\n", fno.fname);               // Print the object name
        fr = f_findnext(&dir, &fno);              // Search for next item 
        xprintf("f_findnext() returns %d\r\n",fr);
        i++;
    }
    return i;
}  


FRESULT parse_BMP_file(char* path)
{
    UINT br;
    if(first_file_parsed_flag == 0)
    {
        fr = f_findfirst(&dir, &fno, "", "*.bmp"); // Start to search for photo files 
        xprintf("f_findfirst() returns %d\r\n",fr);
        first_file_parsed_flag = 1;
    }
    else
    {
        fr = f_findnext(&dir, &fno); // Onto next photo file
        xprintf("f_findnext() returns %d\r\n",fr);
    }

    fr = f_open(&file,fno.fname, FA_READ);
    xprintf("f_open completed and returned %d\r\n",fr);

    if (fr == FR_OK) // if we open the file successfully
    {
        fr= f_read(&file, (void*)&magic, 2, &br); 
        xprintf("Magic #'s are %c%c\r\n",magic.magic[0],magic.magic[1]);
        fr = f_read(&file, (void *) &header , sizeof(header), &br);
        xprintf("file size %d offset %d\n", header.filesz, header.bmp_offset);
        fr= f_read(&file, (void *) &info , sizeof(info),&br);
        xprintf("Width %d, Height %d, bitspp %d\n compression %d\r\n", 
            info.width ,info.height , info.bitspp,info.compress_type);
        fr= f_close(&file);
        return fr;
    }

    else //end of directory - fno.fname is NULL
    {
        first_file_parsed_flag == 0;
        return fr;
    }

}

static uint16_t convert24to16bitcolor(uint8_t R, uint8_t G, uint8_t B)
{
    // In 24 bit color, each RGB param is a BYTE 
    // RRRR RRRR GGGG GGGG BBBB BBBB

    uint16_t retval;

    // shift for 5-6-5 (16 bit)
    R = R>>3; // RRRR RRRR -> 000R RRRR
    G = G>>2; // GGGG GGGG -> 00GG GGGG
    B = B>>3; // BBBB BBBB -> 000B BBBB
    
    // combine
    retval = B;               // 0000 0000 000B BBBB
    retval = (retval<<6) | G; // 0000 0BBB BBGG GGGG
    retval = (retval<<5) | R; // BBBB BGGG GGGR RRRR

    return retval;

}

FRESULT get_BMP_image(char* path)
{
    UINT br; // read count
    uint16_t pixel_16_bit; // 16 bit pixel container  

    xprintf("File is : %s\r\n", fno.fname);

    fr = f_open(&file,fno.fname, FA_READ);
    xprintf("f_open completed and returned %d\r\n",fr);

    if (fr == FR_OK) //we good
    {
        // go to image data
        fr = f_lseek(&file,header.bmp_offset);

        // massive conditional to check that our file complies
        if(info.height == 160 && info.width == 128 && info.bitspp == 24 && info.compress_type == 0)
        {
            ST7735_setAddrWindow(0, 0, ST7735_WIDTH-1,ST7735_HEIGHT-1,MADCTLBMP);
            for (uint8_t x = 0; x<ST7735_HEIGHT; x++)
            {
                for(uint8_t y = 0; y<ST7735_WIDTH; y++)
                { 
                    fr = f_read(&file, (void *) &pixel,sizeof(pixel),&br);
                    // xprintf("B char is %X\r\n",pixel.b);
                    // xprintf("G char is %X\r\n",pixel.g);
                    // xprintf("R char is %X\r\n",pixel.r);
                    pixel_16_bit = convert24to16bitcolor(pixel.r,pixel.g,pixel.b);
                    // xprintf("16 bit conversion returned %X\r\n",pixel_16_bit);
                    ST7735_pushColor(&pixel_16_bit,1);  
                }
            }
        }
        else
        {
            xprintf("ERROR! Image %c Formatting INVALID!\r\n",fno.fname);
        }
    }
    fr= f_close(&file);
    return(fr);
}


FRESULT get_BMP_image_DMA(char* path, uint16_t *byte_buffer)
{
    UINT br; // read count
    uint16_t pixel_16_bit; // 16 bit pixel container  

    xprintf("File is : %s\r\n", fno.fname);

    fr = f_open(&file,fno.fname, FA_READ);
    xprintf("f_open completed and returned %d\r\n",fr);

    if (fr == FR_OK) //we good
    {
        // go to image data
        fr = f_lseek(&file,header.bmp_offset);

        // massive conditional to check that our file complies
        if(info.height == 160 && info.width == 128 && info.bitspp == 24 && info.compress_type == 0)
        {
            // set our window
            ST7735_setAddrWindow(0, 0, ST7735_WIDTH-1,ST7735_HEIGHT-1,MADCTLBMP);
            uint32_t n = 0;
            // while we have bmp bytes to write 
            xprintf("file size is %d\r\n", header.filesz);
            xprintf("bmp offset is %d\r\n",header.bmp_offset);

            while( n < (ST7735_WIDTH*ST7735_HEIGHT))
            {
                // if the remaining bytes to be written to the screen 
                // is >= the buffer size used to write to the screen 
                if(((ST7735_HEIGHT*ST7735_WIDTH)-n) >= sizeof(byte_buffer))
                {
                    for(uint16_t i=0; i<sizeof(byte_buffer); i++) // pack our buffer
                    {
                        fr = f_read(&file, (void *) &pixel,sizeof(pixel),&br);
                        // xprintf("B char is %X\r\n",pixel.b);
                        // xprintf("G char is %X\r\n",pixel.g);
                        // xprintf("R char is %X\r\n",pixel.r);
                        pixel_16_bit = convert24to16bitcolor(pixel.r,pixel.g,pixel.b);
                        // xprintf("16 bit conversion returned %X\r\n",pixel_16_bit);
                        byte_buffer[i] = pixel_16_bit;
                        n++;
                    }
                    ST7735_pushColor(byte_buffer,sizeof(byte_buffer));
                    // 0 out the buffer after each use
                    // Maybe a little crazy, but whatever 
                    memset(byte_buffer,0,sizeof(byte_buffer)); 
                }
                else
                {
                    fr = f_read(&file, (void *) &pixel,sizeof(pixel),&br);
                    // xprintf("B char is %X\r\n",pixel.b);
                    // xprintf("G char is %X\r\n",pixel.g);
                    // xprintf("R char is %X\r\n",pixel.r);
                    pixel_16_bit = convert24to16bitcolor(pixel.r,pixel.g,pixel.b);
                    // xprintf("16 bit conversion returned %X\r\n",pixel_16_bit);
                    ST7735_pushColor(&pixel_16_bit,1);
                    n++;
                }
            }
        }
        else
        {
            xprintf("ERROR! Image %s Formatting INVALID!\r\n",fno.fname);
        }
    }
    
    fr= f_close(&file);
    return(fr);
}


