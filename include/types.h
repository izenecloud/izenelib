#ifndef TYPES_H
#define TYPES_H

#if !defined(WIN32) || defined(__MINGW32__)

#include <stddef.h>
#include <sys/types.h>
#include <stdint.h>

typedef int                 int128_t    __attribute__((mode(TI)));
typedef unsigned int        uint128_t   __attribute__((mode(TI)));

#else

#include <sys/types.h>
#include <wchar.h>

typedef signed char         int8_t;
typedef short               int16_t;
typedef long                int32_t;
typedef __int64             int64_t;
typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned long       uint32_t;
typedef unsigned __int64    uint64_t;

#endif //end of WIN32

#define NS_IZENELIB_BEGIN namespace izenelib{
#define NS_IZENELIB_END }

#define NS_IZENELIB_AM_BEGIN namespace izenelib{ namespace am{
#define NS_IZENELIB_AM_END }}

#define NS_IZENELIB_IR_BEGIN namespace izenelib{ namespace ir{
#define NS_IZENELIB_IR_END }}

#define NS_IZENELIB_UTIL_BEGIN namespace izenelib{ namespace util{
#define NS_IZENELIB_UTIL_END }}

#define BEGIN_SERIALIZATION namespace izenelib{ namespace am{ namespace util{
#define END_SERIALIZATION }}}

#include <cassert>
#include <stdexcept>
#define IASSERT(exp)\
  {              \
    if (!(exp))    \
    {assert(false);}\
  }

#define THROW(exp, info)\
  {              \
    if (!(exp))    \
    {throw (info);}\
  }

#define DISALLOW_COPY_AND_ASSIGN(TypeName)      \
    TypeName(const TypeName&);					\
    void operator=(const TypeName&)

#endif // end of TYPES_H
