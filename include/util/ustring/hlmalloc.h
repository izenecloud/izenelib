#ifndef HLMALLOC_H_
#define HLMALLOC_H_

#include <types.h>

class HLmemory
{
public:
	static void* hlmalloc(uint32_t bytes)
	{
		return malloc(bytes);
	}
};
#endif
