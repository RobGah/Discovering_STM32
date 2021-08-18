#include <stdint.h>
#include <string.h>
#include "ff.h"
#include "xprintf.h"
#include "sdcard.h"

FRESULT scan_files(char* path)
{
    // if you want the default directory just pass in "" for path 
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;

    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) 
            {
                break;  /* Break on error or end of dir */
            }
            if (fno.fattrib & AM_DIR)  /* It is a directory */
            {                   
                i = strlen(path);
                xsprintf(&path[i], "/%s", fno.fname); /* xprintf.h is a hero */
                res = scan_files(path);                    /* Enter the directory */
                if (res != FR_OK) 
                {
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

UINT find_bmp_files(void)
{
    FRESULT fr;     /* Return value */
    DIR dj;         /* Directory object */
    FILINFO fno;    /* File information */
    UINT i = 0;     /* Retval for count of BMPs*/
    fr = f_findfirst(&dj, &fno, "", "?*.bmp"); /* Start to search for photo files */
    if (fr != FR_OK)
    {
        return i;
    }
    i++; // found 1st? ++

    while (fr == FR_OK && fno.fname[0]) {         /* Repeat while an item is found */
        xprintf("%s\n", fno.fname);                /* Print the object name */
        fr = f_findnext(&dj, &fno);               /* Search for next item */
        i++;
    }

    f_closedir(&dj);
    return i;
}  


