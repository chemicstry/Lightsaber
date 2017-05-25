#include "WAV.h"

#include "diag/Trace.h"
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

WAV::WAV(char* file) : _chunkNum(0)
{
    // open file
    f_open(&_fil, file, FA_READ);

    // Read chunk descriptor
    f_read(&_fil, &_riffHeader, sizeof(_riffHeader), &_bytesRead);

    // Read all chunks
    while(!f_eof(&_fil) && _chunkNum < MAX_CHUNKS)
    {
        Chunk& chunk = _chunks[_chunkNum++];
        f_read(&_fil, &chunk.Header, sizeof(ChunkHeader), &_bytesRead);
        chunk.Position = f_tell(&_fil);

        // Seek to next chunk
        f_lseek(&_fil, chunk.Position + chunk.Header.ChunkSize);
    }

    for (int i = 0; i < _chunkNum; ++i)
    {
        if (strncmp(_chunks[i].Header.ChunkID, "fmt ", 4) == 0)
        {
            f_lseek(&_fil, _chunks[i].Position);
            f_read(&_fil, &_formatChunk, sizeof(FormatChunk), &_bytesRead);
        }
        else if (strncmp(_chunks[i].Header.ChunkID, "data", 4) == 0)
        {
            _dataPointer = _chunks[i].Position;
            _dataSize = _chunks[i].Header.ChunkSize;
        }
    }

    // Seek to data chunk
    f_lseek(&_fil, _dataPointer);

    volume = 4;
}

WAV::~WAV()
{
    f_close(&_fil);
}

void WAV::Read(void* buf, uint32_t size)
{
    //uint32_t i = 0;
    f_read(&_fil, buf, MIN(size, _dataSize - (f_tell(&_fil) - _dataPointer)), &_bytesRead);
    /*for (; i < _bytesRead; ++i)
        ((uint8_t*)buf)[i] = ((uint8_t*)buf)[i] >> volume;*/

    if (f_tell(&_fil) >= _dataPointer + _dataSize)
    {
        // Return pointer
        f_lseek(&_fil, _dataPointer);

        f_read(&_fil, (uint8_t*)buf+_bytesRead, size-_bytesRead, &_bytesRead);
        /*for (; i < size; ++i)
            ((uint8_t*)buf)[i] = ((uint8_t*)buf)[i] >> volume;*/
    }

    for (uint32_t i = 0; i < size/2; ++i)
        ((uint16_t*)buf)[i] = (uint16_t)((int32_t)((int16_t*)buf)[i] + 0x7FFF) >> volume;

    /*for (uint32_t i = 0; i < size; ++i)
        ((uint8_t*)buf)[i] = ((uint8_t*)buf)[i] >> volume;*/

    /*for (int i = 0; i < size/2; i += 64)
        trace_printf("%u ", ((uint16_t*)buf)[i]);*/
}
