#include <stdio.h>
#include <math.h>
#include <string>
#include <iostream>
#include "wav.h"

using namespace std;

#define ARRSZ(arr) (sizeof((arr))/sizeof((arr)[0]))
#define MAX(x,y) (x<y) ? (y) : (x);

#define MAX_BPM 220
#define DB_PER_SEC -24

static void printUsage() {
  cout << "Usage: bb_timestamp_gen.exe <inputfile.wav> <outputfile.csv>" << endl;
}

// Usage inputfile outputfile
int main(int argc, char *argv[])
{
  WavReadT wav;
  WavWriteT wavOut;

  // Check for valid arguments.
  if (argc < 3) {
    cout << "Expecting at least 2 arguments." << endl;
    printUsage();
    return -1;
  }

  // Open the output text file.
  FILE *fout = fopen(argv[2], "w");
  if (NULL == fout) {
    cout << "Could not open output file " << argv[2] << endl;
    printUsage();
    return -1;
  }

  // Open the input file.
  WavReadStart( &wav, argv[1] );
  if (!WavReadIsOpen( &wav ) ) {
    cout << "Could not open input file " << argv[1] << endl;
    printUsage();
    fclose(fout);
    return -1;
  }

  // Open an output file for checking the csv file against.
  string outFileName = argv[2];
  outFileName += ".wav";
  WavWriteStart( &wavOut, 1, wav.fs, wav.nBits, outFileName.c_str() );

  fprintf(fout, "filename:%s;FS=%d\n", argv[2], wav.fs);
  fprintf(fout, "timestamp(ms);timestamp(samp);amplitude\n", argv[2]);

  const double linDecPerSec = pow( 10.0, DB_PER_SEC/20 );
  const double linDecPerSamp = pow(10, log10(linDecPerSec) / wav.fs);
  const double hysterises = 1.15;
  
  const double secsPerSample = 1.0 / wav.fs;
  const double maxBeatsPerSec = MAX_BPM / 60.0;
  
  const int minSamplesPerBeat = (int)(0.5 + (wav.fs / maxBeatsPerSec));
  

  // Two versions of timestamps, one in seconds and one in samples.
  double timestampS = 0;
  unsigned int    timeStampSamps = 0;

  // State variables
  bool keepGoing = true;
  bool ampIsUnderEnvelope = true;
  const double minEnvelope = 0.18;
  unsigned int lastTriggerSamp = 0;
  double envelope = 0.9;

  while (keepGoing) {
    int16_t rdbuf[128];
    double  sampbuf[ARRSZ(rdbuf)];
    unsigned int numRead = WavRead16( &wav, rdbuf, ARRSZ(rdbuf) );
    unsigned int numWritten = 0;

    // Deinterleave into the output buffer.
    if (wav.nCh == 1) {
      numWritten = numRead;
      for(int i = 0; i < numWritten; i++) {
        sampbuf[i] = rdbuf[i]*1.0f/32768.0f;
      }
    }
    else if (wav.nCh == 2) {
      numWritten = numRead/2;
      for(int i = 0; i < numWritten; i++) {
        sampbuf[i]  = rdbuf[2*i+0]*1.0f/32768.0f;
        sampbuf[i] += rdbuf[2*i+1]*1.0f/32768.0f;
      }
    }
    else {
      cout << "This program can only handle 1 or two channel .wav files, but the input file has " << wav.nCh << "." << endl;
      return -1;
    }

    // For each sample, track against the current envelope, using hysterises
    for (int i = 0; i < numWritten; i++) {
      const double amp = fabs(sampbuf[i]);
      const double threshold = (ampIsUnderEnvelope) ? envelope * hysterises : envelope / hysterises;
      unsigned int sampDiff = timeStampSamps - lastTriggerSamp;
      if (sampDiff < minSamplesPerBeat) {
        int16_t x = 0;
        WavWrite16( &wavOut, &x, 1 );
      }
      else {
        if (ampIsUnderEnvelope && (amp > threshold)) {
          envelope = amp;
          ampIsUnderEnvelope = false;
          // Trigger here!
          fprintf(fout, "%f;%d;%f\n", timestampS*1000.0, timeStampSamps, amp );
          int16_t x = 32767;
          WavWrite16( &wavOut, &x, 1 );
          lastTriggerSamp = timeStampSamps;
        }
        else {
          if (!ampIsUnderEnvelope && (amp < threshold))  {
            ampIsUnderEnvelope = true;
          }
          int16_t x = 0;
          WavWrite16( &wavOut, &x, 1 );
        }
      }
      envelope *= linDecPerSamp;
      envelope = MAX(minEnvelope, envelope );
    }

    timeStampSamps += numWritten;
    timestampS += (numWritten * secsPerSample); 

    keepGoing = (numRead == ARRSZ(rdbuf));
  }
  fclose(fout);
  WavReadFinish(&wav);

  WavWriteFinish( &wavOut );

  return 0;

}
