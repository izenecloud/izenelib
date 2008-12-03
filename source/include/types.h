#ifndef TYPES_H
#define TYPES_H

#ifndef WIN32
#include <stddef.h>
#include <sys/types.h>
#else
#include <sys/types.h>
#include <wchar.h>
typedef signed char 		int8_t;
typedef short			int16_t;
typedef long				int32_t;
typedef __int64 			int64_t;
typedef unsigned char		uint8_t;
typedef unsigned short	uint16_t;
typedef unsigned long		uint32_t;
typedef unsigned __int64	uint64_t;

#endif //end of WIN32

#endif // end of TYPES_H
