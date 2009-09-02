#ifndef _COMPOUND_KEY_H_
#define _COMPOUND_KEY_H_

#include <am/concept/DataType.h>

NS_IZENELIB_AM_BEGIN

template <typename Key1, typename Key2>
class CompoundKey
{
typedef CompoundKey<Key1, Key2> ThisType;

public:
    CompoundKey(){}

    CompoundKey(const Key1& k1, const Key2& k2)
    :   key1(k1), key2(k2) { }

    int compare(const ThisType& other) const
    {
        if( key1 < other.key1 )
            return -1;
        if( key1 > other.key1 )
            return 1;
        if( key2 < other.key2 )
            return -1;
        if( key2 > other.key2 )
            return 1;
        return 0;
    }

    template<class Archive> void serialize(Archive& ar,
        const unsigned int version)
    {
        ar&key1;
        ar&key2;
    }

    Key1 key1;
    Key2 key2;
};

NS_IZENELIB_AM_END

#define DECLARE_MEMCPY_KEY(type1, type2) \
typedef izenelib::am::CompoundKey<type1,type2> CompoundKey##type1##_##type2; \
MAKE_MEMCPY_TYPE(CompoundKey##type1##_##type2);

/**
 * Key2 is int8_t
 */

DECLARE_MEMCPY_KEY(int8_t, int8_t)
DECLARE_MEMCPY_KEY(uint8_t, int8_t)
DECLARE_MEMCPY_KEY(int16_t, int8_t)
DECLARE_MEMCPY_KEY(uint16_t, int8_t)
DECLARE_MEMCPY_KEY(int32_t, int8_t)
DECLARE_MEMCPY_KEY(uint32_t, int8_t)
DECLARE_MEMCPY_KEY(int64_t, int8_t)
DECLARE_MEMCPY_KEY(uint64_t, int8_t)

/**
 * Key2 is uint8_t
 */

DECLARE_MEMCPY_KEY(int8_t, uint8_t)
DECLARE_MEMCPY_KEY(uint8_t, uint8_t)
DECLARE_MEMCPY_KEY(int16_t, uint8_t)
DECLARE_MEMCPY_KEY(uint16_t, uint8_t)
DECLARE_MEMCPY_KEY(int32_t, uint8_t)
DECLARE_MEMCPY_KEY(uint32_t, uint8_t)
DECLARE_MEMCPY_KEY(int64_t, uint8_t)
DECLARE_MEMCPY_KEY(uint64_t, uint8_t)

/**
 * Key2 is int16_t
 */

DECLARE_MEMCPY_KEY(int8_t, int16_t)
DECLARE_MEMCPY_KEY(uint8_t, int16_t)
DECLARE_MEMCPY_KEY(int16_t, int16_t)
DECLARE_MEMCPY_KEY(uint16_t, int16_t)
DECLARE_MEMCPY_KEY(int32_t, int16_t)
DECLARE_MEMCPY_KEY(uint32_t, int16_t)
DECLARE_MEMCPY_KEY(int64_t, int16_t)
DECLARE_MEMCPY_KEY(uint64_t, int16_t)

/**
 * Key2 is uint16_t
 */

DECLARE_MEMCPY_KEY(int8_t, uint16_t)
DECLARE_MEMCPY_KEY(uint8_t, uint16_t)
DECLARE_MEMCPY_KEY(int16_t, uint16_t)
DECLARE_MEMCPY_KEY(uint16_t, uint16_t)
DECLARE_MEMCPY_KEY(int32_t, uint16_t)
DECLARE_MEMCPY_KEY(uint32_t, uint16_t)
DECLARE_MEMCPY_KEY(int64_t, uint16_t)
DECLARE_MEMCPY_KEY(uint64_t, uint16_t)

/**
 * Key2 is int32_t
 */

DECLARE_MEMCPY_KEY(int8_t, int32_t)
DECLARE_MEMCPY_KEY(uint8_t, int32_t)
DECLARE_MEMCPY_KEY(int16_t, int32_t)
DECLARE_MEMCPY_KEY(uint16_t, int32_t)
DECLARE_MEMCPY_KEY(int32_t, int32_t)
DECLARE_MEMCPY_KEY(uint32_t, int32_t)
DECLARE_MEMCPY_KEY(int64_t, int32_t)
DECLARE_MEMCPY_KEY(uint64_t, int32_t)

/**
 * Key2 is uint32_t
 */

DECLARE_MEMCPY_KEY(int8_t, uint32_t)
DECLARE_MEMCPY_KEY(uint8_t, uint32_t)
DECLARE_MEMCPY_KEY(int16_t, uint32_t)
DECLARE_MEMCPY_KEY(uint16_t, uint32_t)
DECLARE_MEMCPY_KEY(int32_t, uint32_t)
DECLARE_MEMCPY_KEY(uint32_t, uint32_t)
DECLARE_MEMCPY_KEY(int64_t, uint32_t)
DECLARE_MEMCPY_KEY(uint64_t, uint32_t)

/**
 * Key2 is int64_t
 */

DECLARE_MEMCPY_KEY(int8_t, int64_t)
DECLARE_MEMCPY_KEY(uint8_t, int64_t)
DECLARE_MEMCPY_KEY(int16_t, int64_t)
DECLARE_MEMCPY_KEY(uint16_t, int64_t)
DECLARE_MEMCPY_KEY(int32_t, int64_t)
DECLARE_MEMCPY_KEY(uint32_t, int64_t)
DECLARE_MEMCPY_KEY(int64_t, int64_t)
DECLARE_MEMCPY_KEY(uint64_t, int64_t)

/**
 * Key2 is uint64_t
 */

DECLARE_MEMCPY_KEY(int8_t, uint64_t)
DECLARE_MEMCPY_KEY(uint8_t, uint64_t)
DECLARE_MEMCPY_KEY(int16_t, uint64_t)
DECLARE_MEMCPY_KEY(uint16_t, uint64_t)
DECLARE_MEMCPY_KEY(int32_t, uint64_t)
DECLARE_MEMCPY_KEY(uint32_t, uint64_t)
DECLARE_MEMCPY_KEY(int64_t, uint64_t)
DECLARE_MEMCPY_KEY(uint64_t, uint64_t)

#endif
