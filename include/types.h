#ifndef TYPES_H
#define TYPES_H

#ifndef WIN32
#include <stddef.h>
#include <sys/types.h>
#include <stdint.h>
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

#define NAMESPACE_IZENELIB_START namespace izenelib{
#define NAMESPACE_IZENELIB_END }

#define NAMESPACE_IZENELIB_AM_START namespace izenelib{ namespace am{
#define NAMESPACE_IZENELIB_AM_END }}

#define NAMESPACE_IZENELIB_IR_START namespace izenelib{ namespace ir{
#define NAMESPACE_IZENELIB_IR_END }}

#define NAMESPACE_IZENELIB_UTIL_START namespace izenelib{ namespace util{
#define NAMESPACE_IZENELIB_UTIL_END }}

#endif // end of TYPES_H
