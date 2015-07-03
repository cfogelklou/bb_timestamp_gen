
#include "wav.h"


#include <string.h>

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#define LE16TOBUF(u8array, num) ( ((u8array)[1] = (uint8_t) ((num)>>8)),    \
                                ( ( u8array)[0] = (uint8_t) (num)) )

#define BUF2LE16(ptr8)  (uint16_t)( ((uint16_t) (((uint8_t*)(ptr8)))[1] << 8) | \
                                    ((uint16_t) (((uint8_t*)(ptr8)))[0] << 0) )

#define LE32TOBUF(u8array, num) ( ((u8array)[3] = (uint8_t) ((num)>>24)),    \
                                  ((u8array)[2] = (uint8_t) ((num)>>16)),    \
                                  ((u8array)[1] = (uint8_t) ((num)>>8 )),    \
                                  ((u8array)[0] = (uint8_t) ((num)>>0 )) )

#define BUFTOLE32(ptr8)  (uint32_t)( ((uint32_t) (((uint8_t*)(ptr8)))[3] << 24) | \
                                     ((uint32_t) (((uint8_t*)(ptr8)))[2] << 16) | \
                                     ((uint32_t) (((uint8_t*)(ptr8)))[1] << 8) | \
                                     ((uint32_t) (((uint8_t*)(ptr8)))[0] << 0) )

//-----------------------------------------------------------------------------
void WavGetHeader(WavHeaderT *pWavHeader, char *szFileName)
{
   FILE *f  = fopen(szFileName,  "rb");
   memset(pWavHeader, 0, sizeof(WavHeaderT));
   //fstream f(szFileName, ios_base::in | ios_base::binary);
   //if (f.is_open())
   if (f)
   {
      (void)fread (pWavHeader, 1, sizeof(WavHeaderT), f);
      //f.read((char *)pWavHeader, sizeof(WavHeaderT));
      fclose(f);
      //f.close();
   }
}

//-----------------------------------------------------------------------------
void WavWriteHeader(FILE *f, int nCh, int fs, int nBits, unsigned int nSamples)
{
   WavHeaderT WavHeader =
   {
      {'R','I','F','F'}, 0, {'W','A','V','E'},  // RIFF  chunk descriptor
      {'f','m','t',' '}, 16, 1, 0, 0,  0, 0, 0, // fmt sub-chunk
      {'d','a','t','a'}, 0                // data  sub-chunk hdr
   };

   unsigned int nBytesPerSample = nBits / 8;
   WavHeader.NumChannels   = (uint16_t)nCh;
   WavHeader.SampleRate = fs;
   WavHeader.BitsPerSample = (uint16_t)nBits;
   WavHeader.ByteRate      = WavHeader.SampleRate *   WavHeader.NumChannels * 
                       nBytesPerSample;
   WavHeader.BlockAlign = (uint16_t)(WavHeader.NumChannels * nBytesPerSample);
   WavHeader.Subchunk2Size = nSamples * WavHeader.NumChannels * nBytesPerSample;
   WavHeader.ChunkSize     = 4   + (8 + WavHeader.Subchunk1Size)  + (8 + WavHeader.Subchunk2Size);
   if (f)
   {
      fwrite(&WavHeader, 1, sizeof(WavHeaderT), f);
      //f->write((char *)&WavHeader, sizeof(WavHeaderT));
   }
   
}

//-----------------------------------------------------------------------------
void WavWriteChangeProps(WavWriteT *pWav, int nCh, int fs, int nBits)
{
   if (pWav->pF)
   {
      pWav->nCh      = nCh;
      pWav->fs    = fs;
      pWav->nBits    = nBits;
   }
}

//-----------------------------------------------------------------------------
void WavWriteStart(WavWriteT *pWav, int   nCh, int fs, int nBits, const char *szOutFile)
{
   memset(pWav, 0,   sizeof(WavWriteT));
   //pWav->pF = new fstream(szOutFile, ios_base::out |   ios_base::binary);//fopen(szOutFile, "wb");
   pWav->pF = fopen(szOutFile,   "wb");
   if (pWav->pF)
   {
      WavWriteChangeProps(pWav, nCh, fs, nBits);
      WavWriteHeader(pWav->pF, pWav->nCh, pWav->fs, pWav->nBits, 0);
   }
}

//-----------------------------------------------------------------------------
unsigned int WavWrite16(WavWriteT *pWav, int16_t *pBuf,  unsigned int nSamples)
{
   unsigned int nLen =  nSamples;
   
   if (pWav->pF)
   {
      switch (pWav->nBits)
      {
         case 8:
            while (nSamples)
            {
               int8_t samp = (int8_t)(*pBuf++ / 256);
               fwrite(&samp, sizeof(int8_t), 1, pWav->pF);
               //pWav->pF->write((char *)&samp, sizeof(int8_t));
               nSamples--;
            }
            break;
         case 16:
            {
              uint8_t buf8[2];
              unsigned int i = 0;
              while(i < nLen)
              {
                LE16TOBUF( buf8, pBuf[i] );
                fwrite(buf8, sizeof(int16_t), 1, pWav->pF);
                ++i;
              }
            }
            nSamples = 0;
            break;
         case 32:
            {
              uint8_t buf8[4];
              unsigned int i = 0;
              while(i < nLen)
              {
                int32_t samp = (int32_t)pBuf[i];
                samp <<= 16;
                LE32TOBUF( buf8, samp );
                fwrite(buf8, 1, 4, pWav->pF);
                ++i;
              }
            }
            nSamples = 0;
            break;
         default:
            break;
      }
      
   }

   nSamples = nLen   - nSamples;
   pWav->nSamples += nSamples;
   return nSamples;
}

//-----------------------------------------------------------------------------
void WavWriteFinish(WavWriteT *pWav)
{
// if (pWav->pF)
// {
//    unsigned int nSamples = pWav->nSamples / pWav->nCh;
//    pWav->pF->seekg(0, std::ios_base::beg);
//    WavWriteHeader(pWav->pF, pWav->nCh, pWav->fs, pWav->nBits, nSamples);
//    pWav->pF->close();
//    delete pWav->pF;
// }
   if (pWav->pF)
   {
      unsigned int nSamples = pWav->nSamples / pWav->nCh;
      rewind(pWav->pF); 
      WavWriteHeader(pWav->pF, pWav->nCh, pWav->fs, pWav->nBits, nSamples);
      fclose(pWav->pF);
   }
}

//-----------------------------------------------------------------------------
int   WavWriteIsOpen(WavWriteT *pWav)
{
   return (pWav->pF !=  NULL);
}

//-----------------------------------------------------------------------------
void WavReadStart(WavReadT *pWav, char *szInFile)
{
   WavHeaderT WavHdr;
   
   memset(pWav, 0,   sizeof(WavWriteT));
   pWav->pF = fopen(szInFile, "rb");
   //pWav->pF = new fstream(szInFile, ios_base::in | ios_base::binary);
   //if (pWav->pF->is_open())
   if (pWav->pF)
   {
	  (void)fread (&WavHdr,   1, sizeof(WavHeaderT), pWav->pF);
      //pWav->pF->read((char *)&WavHdr, sizeof(WavHeaderT));
      pWav->fs = WavHdr.SampleRate;
      pWav->nBits = WavHdr.BitsPerSample;
      pWav->nCh = WavHdr.NumChannels;
      pWav->ChunkSize   = WavHdr.ChunkSize;
      pWav->ChunkSize   -= 36;
      //pWav->ChunkSize -= (sizeof(WavHeaderT) - 2);
   }
   
}

//-----------------------------------------------------------------------------
unsigned int WavRead16(WavReadT *pWav, int16_t *pBuf,  unsigned int nSamples)
{
   unsigned int nLen =  nSamples;
   size_t nReadBytes = 0;
   if (pWav->pF)
   {
      while (nSamples)
      {
         if (pWav->ChunkSize)
         {
            switch(pWav->nBits)
            {
               case 8:
               {
                  int8_t buf;
                  nReadBytes = fread(&buf, sizeof(int8_t), 1,  pWav->pF);
                  *pBuf++  = (int16_t)(buf   * 256);
                  break;
               }
               case 16:
               {
                  uint8_t buf8[2];
                  nReadBytes = fread(buf8, 1, 2, pWav->pF);
                  *pBuf++ = (int16_t)BUF2LE16( buf8 );
                  break;
               }
               case 32:
               {
                  int32_t samp;
                  uint8_t buf8[4];
                  nReadBytes = fread(buf8, 1, 4, pWav->pF);
                  samp = (int32_t)BUFTOLE32( buf8 );
                  *pBuf++ = (int16_t)(samp / 65536);
                  break;
               }
               default:
               break;
            }
            if (!(nReadBytes)) goto wavread16_filedone;
            pWav->ChunkSize      -= (pWav->nBits/8);
            nSamples -= 1;
         }
         else
         {
            WavHeaderT WavHdr;
            nReadBytes = fread (&WavHdr, sizeof(WavHeaderT), 1,   pWav->pF);
            if (!(nReadBytes)) goto wavread16_filedone;
            //pWav->pF->read((char *)&WavHdr, sizeof(WavHeaderT));
            pWav->ChunkSize   = WavHdr.ChunkSize;
         }
      }
   }
   return nLen - nSamples;
   
wavread16_filedone:
   //if (pWav->pF->is_open())
   if (pWav->pF)
   {
      fclose(pWav->pF);
   }
   pWav->pF = NULL;
   return nLen - nSamples;
   
}

//-----------------------------------------------------------------------------
void WavReadFinish(WavReadT   *pWav)
{
   if (pWav->pF)
   {
      fclose(pWav->pF);
      pWav->pF = NULL;
   }
}

//-----------------------------------------------------------------------------
int   WavReadIsOpen(WavReadT *pWav)
{
   return (pWav->pF !=  NULL);
}

static uint32_t LEtoHost32(const uint8_t* ptr) 
{
    return (uint32_t)( ((uint32_t) *(ptr+3) << 24)   | \
                  ((uint32_t) *(ptr+2) << 16) | \
                  ((uint32_t) *(ptr+1) << 8)  | \
                  ((uint32_t) *(ptr)) );
}

static void StoreLE16(uint8_t *buff, uint16_t le_value) 
{
   buff[1] = (uint8_t)(le_value>>8);
   buff[0] = (uint8_t)le_value;
}

static void StoreLE32(uint8_t *buff, uint32_t le_value)
{
   buff[3] = (uint8_t) (le_value>>24);
   buff[2] = (uint8_t) (le_value>>16);
   buff[1] = (uint8_t) (le_value>>8);
   buff[0] = (uint8_t) le_value;
}

/*=======================================================================*/ /*
    Description: See description at top of file
*/ /*========================================================================*/
void WAV_CreateWaveFile16(WavFile_t *pWav, char *fileNameOut, uint16_t numChannels, uint32_t sampFreq)
{
    uint32_t   chunk1Size = 0;
    uint8_t    longBuff[4];

    /* Open the file name a specified */
    pWav->pFile = fopen(fileNameOut, "wb");
    if (!pWav->pFile) {
        return;
    }

    /**** Create the WAV header ****/
    if (fwrite("RIFF", 1, 4, pWav->pFile) != 4) {
        goto failure;
    }

    /* Skip chunk size for now */
    fseek(pWav->pFile, 8, 0);

    /* Write format */
    if (fwrite("WAVE", 1, 4, pWav->pFile) != 4) {
        goto failure;
    }

    /**** Start Chunk 1 (Format) ****/
    if (fwrite("fmt ", 1, 4, pWav->pFile) != 4) {
        goto failure;
    }

    /* Chunk size */
    chunk1Size = 16L;
    StoreLE32(longBuff, chunk1Size);
    if (fwrite(longBuff, 1, 4, pWav->pFile) != 4) {
        goto failure;
    }

    /* PCM Uncompressed */
    StoreLE16(longBuff, 1);
    if (fwrite(longBuff, 1, 2, pWav->pFile) != 2) {
        goto failure;
    }

    /* Channels */
    StoreLE16(longBuff, numChannels);
    if (fwrite(longBuff, 1, 2, pWav->pFile) != 2) {
        goto failure;
    }

    StoreLE32(longBuff, sampFreq);
    if (fwrite(longBuff, 1, 4, pWav->pFile) != 4) {
        goto failure;
    }

    /* Byte rate */
    StoreLE32(longBuff, sampFreq * numChannels * 16 / 8);
    if (fwrite(longBuff, 1, 4, pWav->pFile) != 4) {
        goto failure;
    }

    /* Block align */
    StoreLE16(longBuff, (uint16_t)(numChannels * 16 / 8));
    if (fwrite(longBuff, 1, 2, pWav->pFile) != 2) {
        goto failure;
    }

    /* Bits per sample */
    StoreLE16(longBuff, 16);
    if (fwrite(longBuff, 1, 2, pWav->pFile) != 2) {
        goto failure;
    }

    /**** Start Chunk 2 (Data) ****/
    fseek(pWav->pFile, chunk1Size + 20, 0);
    if (fwrite("data", 1, 4, pWav->pFile) != 4) {
        goto failure;
    }

    /**** Data chunk ****/
    fseek(pWav->pFile, chunk1Size + 28, 0);

    pWav->len = 0;

    return;


failure:
    if (pWav->pFile)
    {
        fclose( pWav->pFile );
    }
    pWav->pFile = 0;
    return;
}



/*=======================================================================*/ /*
    Description: See description at top of file
*/ /*========================================================================*/
void WAV_CompleteWaveFile16(WavFile_t *pWav)
{
    if ((pWav) && (pWav->pFile))
    {
        uint8_t    longBuff[4];

        /* Write the chunk sizes */
        fseek(pWav->pFile, 4, 0);
        StoreLE32(longBuff, 16L + pWav->len + 20);
        fwrite(longBuff, 1, 4, pWav->pFile);
        fseek(pWav->pFile, 40, 0);
        StoreLE32(longBuff, pWav->len);
        fwrite(longBuff, 1, 4, pWav->pFile);

        fclose(pWav->pFile);
        pWav->pFile = 0;
        pWav->len = 0;
    }
}



/*=======================================================================*/ /*
    Description: See description at top of file
*/ /*========================================================================*/
uint32_t WAV_WriteWaveFile16(WavFile_t *pWav, uint8_t *buffer, uint32_t size)
{
    if ((pWav) && (pWav->pFile))
    {
        uint32_t written = (uint32_t)fwrite(buffer, 1, size, pWav->pFile);
        pWav->len += written;
        if (size != written)
        {
            //BGLOG_WARNING("Could not write all bytes to the wav file!");
        }
        return written;
    }
    else
    {
        return 0;
    }
}


//lint -restore
