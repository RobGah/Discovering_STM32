#include <stdint.h>
#include <string.h>
#include "ff.h"
#include "xprintf.h"
#include "sdcard.h"
#include <stdbool.h>
#include "bmp.h"



FRESULT scan_files(char* path)
{
    // if you want the default directory just pass in "" for path 
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;

    res = f_opendir(&dir, path);
    xprintf("f_opendir() returns %d\r\n",res);
    
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            xprintf("f_readdir() returns %d\r\n",res);
            if (res != FR_OK || fno.fname[0] == 0) 
            {
                xprintf("breaking 1!\r\n");
                break;  /* Break on error or end of dir */
            }
            if (fno.fattrib & AM_DIR)  /* It is a directory */
            {                   
                i = strlen(path);
                xsprintf(&path[i], "/%s", fno.fname); /* xprintf.h is a hero */
                res = scan_files(path);                    /* Enter the directory */
                if (res != FR_OK) 
                {
                    xprintf("breaking 2!\r\n");
                    break;
                }
                path[i] = 0;
            } 
            else 
            {                                       /* It is a file. */
                xprintf("%s/%s\n", path, fno.fname);
            }
        }
        f_closedir(&dir);
    }

    return res;
} 

UINT scan_bmp_files(char* path)
{
    FRESULT fr;     /* Return value */
    DIR dj;         /* Directory object */
    FILINFO fno;    /* File information */
    UINT i = 0;     /* Retval for count of BMPs*/

    fr = f_opendir(&dj, path);
    xprintf("f_opendir() returns %d\r\n",fr);
    fr = f_findfirst(&dj, &fno, "", "*.bmp"); /* Start to search for photo files */
    xprintf("f_findfirst() returns %d\r\n",fr);
    if (fr != FR_OK)
    {
        return i;
    }

    while (fr == FR_OK && fno.fname[0]) {         /* Repeat while an item is found */
        xprintf("%s\n", fno.fname);               /* Print the object name */
        fr = f_findnext(&dj, &fno);              /* Search for next item */
        xprintf("f_findnext() returns %d\r\n",fr);
        i++;
    }

    f_closedir(&dj);
    return i;
}  


FRESULT parse_BMP_file(char* path)
{
    FIL file;
    FRESULT fr;
    DIR dj;         /* Directory object */
    FILINFO fno;    /* File information */
    UINT br;         /* read count */

    
    fr = f_opendir(&dj, path);
    xprintf("f_opendir() returns %d\r\n",fr);
    fr = f_findfirst(&dj, &fno, "", "*.bmp"); /* Start to search for photo files */
    xprintf("f_findfirst() returns %d\r\n",fr);

    fr = f_open(&file,fno.fname, FA_READ);
    xprintf("f_open completed and returned %d\r\n",fr);
    if (fr == FR_OK) //we good
    {
        fr= f_read(&file, (void*)&magic, 2, &br); 
        xprintf("Magic #'s are %c%c\r\n",magic.magic[0],magic.magic[1]);
        fr = f_read(&file, (void *) &header , sizeof(header), &br);
        xprintf("file size %d offset %d\n", header.filesz, !header.bmp_offset);
        fr= f_read(&file, (void *) &info , sizeof(info),&br);
        xprintf("Width %d, Height %d, bitspp %d\n", info.width ,info.height , info.bitspp);
        fr= f_close(&file);
    }

    f_closedir(&dj);


}




