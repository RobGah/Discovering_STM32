#include <stdint.h>
#include <string.h>

#define ISMAIN

#ifdef ISMAIN
#include <stdio.h> 
#include <fcntl.h>
#endif

#define RIFF 'FFIR'
#define WAVE 'EVAW'
#define fmt ' tmf'
#define data 'atad'

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

struct wav_data
{
    uint32_t FormatTag; // 'data'
    uint32_t SubchunkSize; //if odd, there will be a pad byte at EoF.
    // SubchunkSize bytes of data follows...
};

#ifdef ISMAIN


int wavfile;

struct RIFF_header riffheader;
struct fmt_chunk fmtchunk;
struct wav_data datachunk;

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
            printf("File is type \"%X\".\r\n",riffheader.ChunkID);
            read(wavfile,(void *)&riffheader.ChunkSize, 4);
            printf("File size is %X.\r\n",riffheader.ChunkSize);
            read(wavfile,(void *)&riffheader.Format, 4);
            printf("Format is \"%X\".\r\n",riffheader.Format);
        }

        else
        {
            printf("Not a WAV file!\r\n");
        }

        read(wavfile,(void *)&fmtchunk.FormatTag,4);
        if(fmtchunk.FormatTag == fmt)
        {
            printf("Format is \"%X\".\r\n",fmtchunk.FormatTag);
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
        close(wavfile);
    return 1;
    }
}

#endif