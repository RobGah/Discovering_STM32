#include <stdint.h>
#include <string.h>
#include <stdbool.h>

//#define ISMAIN

#ifdef ISMAIN
#include <stdio.h> 
#include <fcntl.h>
#endif

#ifndef ISMAIN
#include "ff.h"
#include "diskio.h"
#include "xprintf.h"
#include "wav_file.h"
#endif

#define RIFF 'FFIR'
#define WAVE 'EVAW'
#define fmt ' tmf'
#define data 'atad'
#define list 'TSIL'
#define info 'OFNI'

// see http://soundfile.sapp.org/doc/WaveFormat/
// easy single file gcc compile : "gcc -g -Wall -owav_file wav_file.c"
// and run w/ "./wav_file.exe"

struct RIFF_header 
{
    uint32_t ChunkID; // 'RIFF'
    uint32_t ChunkSize; // entire file size minus the 8 bytes for ChunkSize and ChunkID
    uint32_t Format; // 'WAVE'
};

struct fmt_chunk
{
    uint32_t FormatTag; // 'fmt'
    uint32_t SubchunkSize;
    uint16_t AudioFormat; // 1 for PCM
    uint16_t nChannels; // 1 for Mono
    uint32_t nSamplesPerSec;
    uint32_t nAvgBytesPerSec;
    uint16_t nBlockAlign; 
    uint16_t wBitsPerSample; // should be 8 for our needs
    // ExtraParamsSize - DNE if PCM.
    // ExtraParams - DNE if PCM.
};

/* 
LIST is an optional chunk in some files
It can be of variable length and if its type INFO (it often is)
it will have artist/album, engineer on record, etc.
 */
struct list_chunk  
{
    uint32_t FormatTag;
    uint32_t chunkID;
    uint32_t SubchunkSize;
};

struct wav_data
{
    uint32_t FormatTag; // 'data'
    uint32_t SubchunkSize; //if odd, there will be a pad byte at EoF.
    // SubchunkSize bytes of data follows...
};

#ifndef ISMAIN
// declare instances of these structs
static struct RIFF_header riffheader;
static struct fmt_chunk fmtchunk;
static struct wav_data datachunk;
static struct list_chunk listchunk;

// FATFS variables
FATFS FatFs;		/* FatFs work area needed for each volume */
FIL wavfile;		/* File object needed for each open file */
UINT bmp_count;     /* ad oculos*/
FRESULT fr;         /* FatFs function common result code*/
DIR dir;
static FILINFO fno;
static bool file_is_open = false;

enum PARSE_ERRORS {
    BAD_WAV_FILE,       // Error opening file
    NOT_RIFF,           // File does not have a RIFF tag
    NOT_WAV_FMT,        // File format is not WAV 
    NOT_FMT_HEADER,     // FMT header not found in WAV file
    NO_WAV_DATA         // File contains no WAV data (likely won't see this ever)
    };


uint32_t parse_wavfile(char * filename)
{
    /*
    Takes a filename within the current directory (specified outside this function)
    and parses the major headers. Fills the declared structs w/ information about 
    the WAV file so that it can be used later. 

    inputs: filename e.g. "sound.wav"
    outputs: a uint32_t value for the offset between the start of the file
    and the large chunk of sound bytes for access w/ f_lseek() in a read function.
    */

    UINT br;
    uint32_t retval = 0;

    if(file_is_open != true)
        {
            fr = f_open(&wavfile, &filename, FA_READ);
            xprintf("f_open completed and returned %d\r\n",fr);
            if(fr != FR_OK)
            {
                xprintf("File not found!\r\n");
                return BAD_WAV_FILE;
            }
            else file_is_open = true;
        }

    fr = f_read(&wavfile,(void *)&riffheader.ChunkID, 4, &br);
       
        if(riffheader.ChunkID == RIFF)
        {
            xprintf("RIFF header ChunkID is type RIFF\r\n");
            fr = f_read(&wavfile,(void *)&riffheader.ChunkSize, 4, &br);
            retval = riffheader.ChunkSize + 8; //+ ChunkID + Chunksize bytes
            xprintf("File size is %X.\r\n",riffheader.ChunkSize);
            fr = f_read(&wavfile,(void *)&riffheader.Format, 4, &br);
            if(riffheader.Format == WAVE)
            {
                xprintf("Format is WAVE.\r\n");
            }
            else
            {
                xprintf("NOT a WAVE File");
                return NOT_WAV_FMT;
            }
        }
        else
        {
            xprintf("Not a RIFF header!\r\n");
            return NOT_RIFF;
        }

        fr= f_read(&wavfile,(void *)&fmtchunk.FormatTag,4,&br);
        if(fmtchunk.FormatTag == fmt)
        {
            xprintf("Subchunk1 ID is fmt. \r\n");
            fr = f_read(&wavfile,(void *)&fmtchunk.SubchunkSize,4,&br);
            xprintf("Format Chunk Size is %X.\r\n",fmtchunk.SubchunkSize);
            fr = f_read(&wavfile,(void *)&fmtchunk.AudioFormat,2,&br);
            xprintf("Audio Format is %X.\r\n",fmtchunk.AudioFormat);
            fr = f_read(&wavfile,(void *)&fmtchunk.nChannels,2,&br);
            xprintf("Number of channels: %X.\r\n",fmtchunk.nChannels);
            fr = f_read(&wavfile,(void *)&fmtchunk.nSamplesPerSec,4,&br);
            xprintf("Samples/sec: %X.\r\n",fmtchunk.nSamplesPerSec);
            fr = f_read(&wavfile,(void *)&fmtchunk.nAvgBytesPerSec,4,&br);
            xprintf("Average Bytes/sec: %X.\r\n",fmtchunk.nAvgBytesPerSec);
            fr = f_read(&wavfile,(void *)&fmtchunk.nBlockAlign,2,&br);
            xprintf("Bytes/sample: %X.\r\n",fmtchunk.nBlockAlign);
            fr = f_read(&wavfile,(void *)&fmtchunk.wBitsPerSample,2,&br);
            xprintf("Bits/sample: %X.\r\n",fmtchunk.wBitsPerSample);
        }
        else
        {
            xprintf("There is no fmt chunk - ERROR\r\n");
            return NOT_FMT_HEADER;
        }
            do
        {
            f_read(&wavfile,(void *)&datachunk.FormatTag,4,br);
        } while (datachunk.FormatTag != data);

        if(datachunk.FormatTag == data)
        {
            xprintf("SubChunk2 ID is DATA\r\n");
            f_read(&wavfile,(void *)&datachunk.SubchunkSize,4,br);
            xprintf("SubChunk2 Size is: %X.\r\n",datachunk.SubchunkSize);
            retval -= datachunk.SubchunkSize;
            //get a full readout - not recommended.
            //printf("Raw data bytes:\r\n");
            // for(uint32_t i = 0; i<datachunk.SubchunkSize;i++)
            // {
            //     read(wavfile,(void *) &audio_data,1);
            //     printf("%X ",audio_data);
                
            //     if(i%10 == 0)
            //     {
            //         printf("\r\n");
            //     }
            // }
        }
        else
        {
            // I don't think we could ever get here.
            xprintf("WAV Data not found! - ERROR \r\n");
            return NO_WAV_DATA;
        }
        xprintf("Byte offset from file start to data: %X.\r\n",retval);
        
        f_close(&wavfile);
        return(retval);
}


FRESULT read_wavfile_data(char * filename, uint8_t *readbuffer, uint32_t offset, uint8_t numbytes)
{
    /*
    -Reads numbytes of data from the specified wavfile into a buffer at the location
        specified by an offset from the start of the wav file. 
    -Can open the file if not open already, but does not close it.
    -Offset is incremented to save the location of the read in case of file closure
        and also to track the progress of the read to determine EOF.

    */
    UINT br; // read count
    if(file_is_open != true)
    {
        fr = f_open(&wavfile, &filename, FA_READ);
        xprintf("f_open completed and returned %d\r\n",fr);
        if(fr != FR_OK)
        {
            xprintf("File open failed!\r\n");
            return fr;
        }
        else file_is_open = true;
    }

        // go to wav data
        fr = f_lseek(&wavfile,offset);
        fr = f_read(&wavfile,&readbuffer,numbytes,br);
        offset+=numbytes; // increment offset?
        return fr;
}

uint32_t get_wavfile_size(char *filename)
{
    parse_wavfile(&filename);
    return riffheader.ChunkSize;
}

FRESULT close_wavfile()
{
    f_close(&wavfile);
}  


#endif 

/***********************************/
/****BEGIN MAIN FILE PARSER .EXE****/
/***********************************/
#ifdef ISMAIN

uint32_t wavfile;
char audio_data;

struct RIFF_header riffheader;
struct fmt_chunk fmtchunk;
struct wav_data datachunk;
struct list_chunk listchunk;

int main(int argc, char * argv[])
{    
    if(argc==1)
    {
        printf("Not enough arguments! Expected a file.\r\n");
        printf("Example: ./wav_file.exe test.wav\r\n");
        return -1;
    }
    
    else if(argc>1)
    {
        printf("File passed is: %s\r\n",argv[1]);

        wavfile = open(argv[1],O_RDONLY);
        if(wavfile == -1)
        {
            printf("ERROR: Can't open file!\r\n");
            return -2;
        }

        read(wavfile,(void *)&riffheader.ChunkID, 4);
        if(riffheader.ChunkID == RIFF)
        {
            printf("RIFF header ChunkID is type RIFF\r\n");
            read(wavfile,(void *)&riffheader.ChunkSize, 4);
            printf("File size is %X.\r\n",riffheader.ChunkSize);
            read(wavfile,(void *)&riffheader.Format, 4);
            if(riffheader.Format == WAVE)
            {
                printf("Format is WAVE.\r\n");
            }
        }

        else
        {
            printf("Not a WAV file!\r\n");
        }

        read(wavfile,(void *)&fmtchunk.FormatTag,4);
        if(fmtchunk.FormatTag == fmt)
        {
            printf("Subchunk1 ID is fmt. \r\n");
            read(wavfile,(void *)&fmtchunk.SubchunkSize,4);
            printf("Format Chunk Size is %X.\r\n",fmtchunk.SubchunkSize);
            read(wavfile,(void *)&fmtchunk.AudioFormat,2);
            printf("Audio Format is %X.\r\n",fmtchunk.AudioFormat);
            read(wavfile,(void *)&fmtchunk.nChannels,2);
            printf("Number of channels: %X.\r\n",fmtchunk.nChannels);
            read(wavfile,(void *)&fmtchunk.nSamplesPerSec,4);
            printf("Samples/sec: %X.\r\n",fmtchunk.nSamplesPerSec);
            read(wavfile,(void *)&fmtchunk.nAvgBytesPerSec,4);
            printf("Average Bytes/sec: %X.\r\n",fmtchunk.nAvgBytesPerSec);
            read(wavfile,(void *)&fmtchunk.nBlockAlign,2);
            printf("Bytes/sample: %X.\r\n",fmtchunk.nBlockAlign);
            read(wavfile,(void *)&fmtchunk.wBitsPerSample,2);
            printf("Bits/sample: %X.\r\n",fmtchunk.wBitsPerSample);
        }

        /* 
        Attempted to parse the LIST portion of the file (if it exists)
        Might revisit, but its unnessessary. 
        */

        // read(wavfile,(void *)&listchunk.FormatTag,4);
        // if(listchunk.FormatTag == list)
        // {
        //     printf("LIST FormatTag is \"%X\".\r\n",listchunk.FormatTag);
        //     read(wavfile,(void *)&listchunk.SubchunkSize,4);
        //     printf("LIST chunk size is: %X\r\n",listchunk.SubchunkSize);
        //     read(wavfile,(void *)&listchunk.chunkID,4);
        //     if (listchunk.chunkID == info)
        //     {
        //         printf("LIST chunk is type INFO.\r\n");
        //         read(wavfile,(void *)&listchunk.chunkID,4);
        //         while(listchunk.chunkID != data)
        //         {
        //             printf("INFO SubChunk ID is \"%X\".\r\n",listchunk.chunkID);
        //             read(wavfile,(void *)&listchunk.SubchunkSize,4);
        //             printf("Size of Info Subchunk is %X\r\n",listchunk.SubchunkSize);
        //             printf("\"%X\"'s data is:\r\n",listchunk.chunkID);
        //             for(uint32_t i = 0; i<listchunk.SubchunkSize;i++)
        //             {
        //                 read(wavfile,(void *) audio_data,1);
        //                 printf("%X",audio_data);
        //             }
        //             printf("\r\n");
        //             read(wavfile,(void *)&listchunk.chunkID,4);
        //         }
        //         // we hit the condition where chunkID is 'data'
        //         // assign this to the datachunk's format tag
        //         datachunk.FormatTag = listchunk.chunkID;
        //     }
        // }
        //// if list chunk DNE, search for data chunk and proceed
        //else
        //

        do
        {
            read(wavfile,(void *)&datachunk.FormatTag,4);
        } while (datachunk.FormatTag != data);

        if(datachunk.FormatTag == data)
        {
            printf("SubChunk2 ID is DATA\r\n");
            read(wavfile,(void *)&datachunk.SubchunkSize,4);
            printf("SubChunk2 Size is: %X.\r\n",datachunk.SubchunkSize);
            //get a full readout - not recommended.
            //printf("Raw data bytes:\r\n");
            // for(uint32_t i = 0; i<datachunk.SubchunkSize;i++)
            // {
            //     read(wavfile,(void *) &audio_data,1);
            //     printf("%X ",audio_data);
                
            //     if(i%10 == 0)
            //     {
            //         printf("\r\n");
            //     }
            // }
        }
        close(wavfile);
    }
    return 1;
}

#endif