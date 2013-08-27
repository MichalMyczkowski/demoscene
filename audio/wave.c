#include <proto/dos.h>

#include "audio/wave.h"
#include "std/debug.h"
#include "std/memory.h"
#include "system/fileio.h"

#define ID_RIFF MAKE_ID('R', 'I', 'F', 'F')
#define ID_WAVE MAKE_ID('W', 'A', 'V', 'E')
#define ID_FMT  MAKE_ID('f', 'm', 't', ' ')
#define ID_FACT MAKE_ID('f', 'a', 'c', 't')
#define ID_DATA MAKE_ID('d', 'a', 't', 'a')

typedef struct {
  int riffId;
  int ckSize;
  int waveId;
} WaveHeaderT;

typedef struct {
  int ckId;
  int ckSize;
} ChunkT;

typedef struct {
  uint16_t wFormatTag;
  uint16_t nChannels; 
  uint32_t nSamplesPerSec; 
  uint32_t nAvgBytesPerSec;
  uint16_t nBlockAlign;
  uint16_t wBitsPerSample;
  uint16_t cbSize;
} __attribute__((packed)) FmtChunkT;

bool WaveFileOpen(WaveFileT *wave, const char *filename) {
  char *path = AbsPath(filename);
  BPTR fh = Open(path, MODE_OLDFILE);

  MemUnref(path);

  if (fh) {
    WaveHeaderT header;
    ChunkT chunk;
    FmtChunkT fmt;
    int dwSampleLength = 0;
    int dataSize = 0;

    Read(fh, &header, sizeof(header));
    header.ckSize = bswap32(header.ckSize);

    if (header.riffId == ID_RIFF && header.waveId == ID_WAVE) {
      while (Seek(fh, 0, OFFSET_CURRENT) < header.ckSize + 8) {
        Read(fh, &chunk, sizeof(chunk));

        if (chunk.ckId == ID_FMT) {
          bool ext_chunk = bswap32(chunk.ckSize) > 16;
          Read(fh, &fmt, ext_chunk ? 18 : 16);
          if (ext_chunk)
            Seek(fh, bswap16(fmt.cbSize), OFFSET_CURRENT);
        } else if (chunk.ckId == ID_FACT) {
          Read(fh, &dwSampleLength, sizeof(dwSampleLength));
        } else if (chunk.ckId == ID_DATA) {
          wave->samplesOffset = Seek(fh, 0, OFFSET_CURRENT);
          dataSize = bswap32(chunk.ckSize);
          Seek(fh, dataSize, OFFSET_CURRENT);
        } else {
          PANIC("Unknown RIFF chunk '%4s'.", (const char *)&chunk);
        }
      }

      if (Seek(fh, 0, OFFSET_CURRENT) == header.ckSize + 8) {
        wave->fh = fh;
        wave->format = bswap16(fmt.wFormatTag);
        wave->channels = bswap16(fmt.nChannels);
        wave->blockAlign = bswap16(fmt.nBlockAlign);
        wave->bitsPerSample = bswap16(fmt.wBitsPerSample);
        wave->samplesPerSec = bswap32(fmt.nSamplesPerSec);

        if (dwSampleLength > 0)
          wave->samplesNum = bswap32(dwSampleLength);
        else
          wave->samplesNum =
            dataSize / (wave->channels * wave->bitsPerSample / 8);

        LOG("File '%s' - fmt: %d, chs: %d, bits: %d, freq: %d.", filename,
            wave->format, wave->channels, wave->bitsPerSample,
            wave->samplesPerSec);

        Seek(fh, wave->samplesOffset, OFFSET_BEGINNING);
        return true;
      }
    } else {
      LOG("File '%s' not in WAVE format.", filename);
    }

    Close(fh);
  } else {
    LOG("File '%s' not found.", filename);
  }

  return false;
}

void WaveFileClose(WaveFileT *wave) {
  Close(wave->fh);
}

void WaveFileChangePosition(WaveFileT *wave, float second) {
  size_t sampleWidth = wave->channels * wave->bitsPerSample / 8;
  size_t offset = wave->samplesOffset +
    (size_t)(wave->samplesPerSec * second) * sampleWidth;

  Seek(wave->fh, offset, OFFSET_BEGINNING);
}

size_t WaveFileReadSamples(WaveFileT *wave, PtrT samples, size_t requested) {
  int sampleWidth = wave->channels * wave->bitsPerSample / 8;
  int obtained;

  requested *= sampleWidth;
  obtained = Read(wave->fh, samples, requested);

  /* Clear rest of buffer if needed. */
  if (obtained < requested)
    memset(samples + obtained, 0, requested - obtained);

  return obtained / sampleWidth;
}
