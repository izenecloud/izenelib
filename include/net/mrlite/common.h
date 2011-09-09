#ifndef BASE_COMMON_H_
#define BASE_COMMON_H_

#include <stdlib.h>
#include <stdint.h>

#include <glog/logging.h>

//-----------------------------------------------------------------------------
//
// This marcro is used to disallow copy constructor and assign operator in
// class definition. For more details, please refer to Google coding style
// document
// [http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml
// #Copy_Constructors]
//
// To use the macro, just put it in private section of class, illustrated as
// the following example.
//
// class Foo {
//  public :
//    Foo();
//  private :
//    DISALLOW_COPY_AND_ASSIGN(Foo);
// };
//
//-----------------------------------------------------------------------------

#define DISALLOW_COPY_AND_ASSIGN(TypeName)      \
    TypeName(const TypeName&);                    \
    void operator=(const TypeName&)

//-----------------------------------------------------------------------------
//
// Basis POD types.
//
//-----------------------------------------------------------------------------

typedef unsigned int uint;

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

static const int32 kInt32Max = 0x7FFFFFFF;
static const int32 kInt32Min = -kInt32Max - 1;
static const int64 kInt64Max = 0x7FFFFFFFFFFFFFFFll;
static const int64 kInt64Min = -kInt64Max - 1;
static const uint32 kUInt32Max = 0xFFFFFFFFu;
static const uint64 kUInt64Max = 0xFFFFFFFFFFFFFFFFull;

#endif  // BASE_COMMON_H_

