#ifndef IZENE_SERIALIZATION_FEBIRD_H_
#define IZENE_SERIALIZATION_FEBIRD_H_

#include "izene_type_traits.h"

#include <3rdparty/febird/io/DataIO.h>
#include <3rdparty/febird/io/StreamBuffer.h>
#include <3rdparty/febird/io/FileStream.h>
#include <3rdparty/febird/io/ConcurrentStream.h>

using namespace febird;
using febird::ulong;
using febird::ushort;

NS_IZENELIB_UTIL_BEGIN

template<typename T>
class izene_serialization_febird
{
    NativeDataOutput<AutoGrownMemIO> oa;
public:
    izene_serialization_febird(const T& dat)
    {
        oa & dat;
    }
    void write_image(char* &ptr, size_t& size)
    {
        ptr = (char*) oa.getStream()->begin();
        size = oa.getStream()->tell();
    }

};

template<typename T>
class izene_deserialization_febird
{
    NativeDataInput<MemIO> ia;
public:
    izene_deserialization_febird(const char* ptr,  const size_t size)
    {
        ia.set((void*) ptr, size);
    }
    void read_image(T& dat)
    {
        ia & dat;
    }
};

NS_IZENELIB_UTIL_END

#endif /*IZENE_SERIALIZATION_FEBIRD_H_*/
