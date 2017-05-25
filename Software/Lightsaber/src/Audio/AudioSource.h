#ifndef _AUDIO_SOURCE_H_
#define _AUDIO_SOURCE_H_

#include "stdint.h"

class AudioSource
{
public:
	virtual ~AudioSource() {}

	virtual void Read(void* buf, uint32_t size) = 0;
	virtual bool EndOfStream() = 0;
};

#endif
