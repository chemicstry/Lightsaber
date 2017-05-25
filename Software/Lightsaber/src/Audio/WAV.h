#ifndef _WAV_H
#define _WAV_H

#include "AudioSource.h"
#include "../fatfs/ff.h"
#include "stdint.h"
#include "string.h"

struct RIFFHeader
{
    char        ChunkID[4];
    uint32_t    ChunkSize;
    char        Format[4];
};

struct ChunkHeader
{
    char        ChunkID[4];
    uint32_t    ChunkSize;
};

struct FormatChunk
{
    uint16_t    AudioFormat;
    uint16_t    Channels;
    uint32_t    SampleRate;
    uint32_t    ByteRate;
    uint16_t    BlockAlign;
    uint16_t    BitsPerSample;
};

#define MAX_CHUNKS 4

struct Chunk
{
    ChunkHeader Header;
    uint32_t    Position;
};

class WAV : public AudioSource
{
public:
    WAV(const char* file, bool loop = false);
    ~WAV();
    FormatChunk* GetFormat()
    {
        return &_formatChunk;
    }

    uint32_t GetSize()
    {
        return _dataSize;
    }

    void Read(void* buf, uint32_t size);

    bool EndOfStream()
    {
    	return _eof;
    }

private:
    FIL _fil;
    UINT _bytesRead;
    uint32_t _dataSize;
    uint32_t _dataPointer;
    RIFFHeader _riffHeader;
    FormatChunk _formatChunk;

    Chunk _chunks[MAX_CHUNKS];
    uint8_t _chunkNum;

    bool _loop, _eof;
};

#endif
