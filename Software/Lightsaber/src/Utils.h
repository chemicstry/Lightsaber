#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdlib.h>

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

#define SWAP_UINT16(x) (((x) >> 8) | ((x) << 8))

inline int RandomInt(int min, int max)
{
	return rand() % (max - min) + min;
}

#endif
