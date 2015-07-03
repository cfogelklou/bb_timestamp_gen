#ifndef   __WAV_H__
#define   __WAV_H__

#include <stdint.h>
#include <stdio.h>
// Functions are C-callable
#ifdef __cplusplus
extern "C" {
#endif

typedef  struct WavHeaderTag
{
   uint8_t     ChunkID[4];
   uint32_t    ChunkSize;
   uint8_t     Format[4];
   uint8_t     Subchunk1ID[4];
   uint32_t    Subchunk1Size;
   uint16_t    AudioFormat;
   uint16_t    NumChannels;
   uint32_t    SampleRate;
   uint32_t    ByteRate;
   uint16_t    BlockAlign;
   uint16_t    BitsPerSample;
   uint8_t     SubChunk2ID[4];
   uint32_t    Subchunk2Size;
} WavHeaderT;

void           WavGetHeader(WavHeaderT *pWavHeader, char *szFileName);
void           WavWriteHeader(FILE *f, int nCh, int fs, int nBits, unsigned int nSamples);

typedef  struct WavWriteTag
{
   FILE     *     pF;
   int            nCh;
   int            fs;
   int            nBits;
   unsigned int   nSamples;
} WavWriteT;

void           WavWriteStart(WavWriteT *pWav, int   nCh, int fs, int nBits, const char *szOutFile);
unsigned int   WavWrite16(WavWriteT *pWav, int16_t *pBuf,  unsigned int nSamples);
void           WavWriteChangeProps(WavWriteT *pWav, int nCh, int fs, int nBits);
void           WavWriteFinish(WavWriteT *pWav);
int            WavWriteIsOpen(WavWriteT *pWav);

typedef  struct WavReadTag
{
   FILE     *     pF;
   int            nCh;
   int            fs;
   int            nBits;
   uint32_t       ChunkSize;
} WavReadT;

void           WavReadStart(WavReadT *pWav, char *szInFile);
unsigned int   WavRead16(WavReadT  *pWav, int16_t *pBuf, unsigned int nSamples);
unsigned int   WavRead16Swap(WavReadT *pWav, int16_t *pBuf, unsigned int nSamples);
void           WavReadFinish(WavReadT   *pWav);
int            WavReadIsOpen(WavReadT *pWav);

#define IANY_WAV
#ifdef IANY_WAV

typedef struct _WavFile_t
{
  FILE  *pFile;
  uint32_t len;
} WavFile_t;
/*=======================================================================*/ /*
    Creates a .wav file.
*/ /*========================================================================*/
void WAV_CreateWaveFile16(WavFile_t *pWav, char *fileNameOut, uint16_t numChannels, uint32_t sampFreq);

/*=======================================================================*/ /*
    Completes a .wav file.
*/ /*========================================================================*/
void  WAV_CompleteWaveFile16(WavFile_t *pWav);

/*=======================================================================*/ /*
    Writes data to a .wav file.
*/ /*========================================================================*/
uint32_t   WAV_WriteWaveFile16(WavFile_t *pWav, uint8_t *buffer, uint32_t size);
#endif

// Functions are C-callable
#ifdef __cplusplus
}
#endif

#endif

