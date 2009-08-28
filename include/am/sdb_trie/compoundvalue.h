#ifndef _COMPOUND_VALUE_H_
#define _COMPOUND_VALUE_H_

#include <am/concept/DataType.h>

NS_IZENELIB_AM_BEGIN

template <typename Value1, typename Value2>
class CompoundValue
{
typedef CompoundValue<Value1, Value2> ThisType;

public:
    CompoundValue(){}

    CompoundValue(const Value1& v1, const Value2& v2, bool t)
    :   value1(v1), value2(v2), tag(t) { }

    template<class Archive> void serialize(Archive& ar,
        const unsigned int version)
    {
        ar&value1;
        ar&value2;
        ar&tag;
    }

    Value1 value1;
    Value2 value2;
    bool tag;
};

NS_IZENELIB_AM_END

#define DECLARE_MEMCPY_VALUE(type1, type2) \
typedef izenelib::am::CompoundValue<type1,type2> CompoundValue##type1##_##type2; \
MAKE_MEMCPY_TYPE(CompoundValue##type1##_##type2);

/**
 * Key2 is int8_t
 */

DECLARE_MEMCPY_VALUE(int8_t, int8_t)
DECLARE_MEMCPY_VALUE(uint8_t, int8_t)
DECLARE_MEMCPY_VALUE(int16_t, int8_t)
DECLARE_MEMCPY_VALUE(uint16_t, int8_t)
DECLARE_MEMCPY_VALUE(int32_t, int8_t)
DECLARE_MEMCPY_VALUE(uint32_t, int8_t)
DECLARE_MEMCPY_VALUE(int64_t, int8_t)
DECLARE_MEMCPY_VALUE(uint64_t, int8_t)

/**
 * Key2 is uint8_t
 */

DECLARE_MEMCPY_VALUE(int8_t, uint8_t)
DECLARE_MEMCPY_VALUE(uint8_t, uint8_t)
DECLARE_MEMCPY_VALUE(int16_t, uint8_t)
DECLARE_MEMCPY_VALUE(uint16_t, uint8_t)
DECLARE_MEMCPY_VALUE(int32_t, uint8_t)
DECLARE_MEMCPY_VALUE(uint32_t, uint8_t)
DECLARE_MEMCPY_VALUE(int64_t, uint8_t)
DECLARE_MEMCPY_VALUE(uint64_t, uint8_t)

/**
 * Key2 is int16_t
 */

DECLARE_MEMCPY_VALUE(int8_t, int16_t)
DECLARE_MEMCPY_VALUE(uint8_t, int16_t)
DECLARE_MEMCPY_VALUE(int16_t, int16_t)
DECLARE_MEMCPY_VALUE(uint16_t, int16_t)
DECLARE_MEMCPY_VALUE(int32_t, int16_t)
DECLARE_MEMCPY_VALUE(uint32_t, int16_t)
DECLARE_MEMCPY_VALUE(int64_t, int16_t)
DECLARE_MEMCPY_VALUE(uint64_t, int16_t)

/**
 * Key2 is uint16_t
 */

DECLARE_MEMCPY_VALUE(int8_t, uint16_t)
DECLARE_MEMCPY_VALUE(uint8_t, uint16_t)
DECLARE_MEMCPY_VALUE(int16_t, uint16_t)
DECLARE_MEMCPY_VALUE(uint16_t, uint16_t)
DECLARE_MEMCPY_VALUE(int32_t, uint16_t)
DECLARE_MEMCPY_VALUE(uint32_t, uint16_t)
DECLARE_MEMCPY_VALUE(int64_t, uint16_t)
DECLARE_MEMCPY_VALUE(uint64_t, uint16_t)

/**
 * Key2 is int32_t
 */

DECLARE_MEMCPY_VALUE(int8_t, int32_t)
DECLARE_MEMCPY_VALUE(uint8_t, int32_t)
DECLARE_MEMCPY_VALUE(int16_t, int32_t)
DECLARE_MEMCPY_VALUE(uint16_t, int32_t)
DECLARE_MEMCPY_VALUE(int32_t, int32_t)
DECLARE_MEMCPY_VALUE(uint32_t, int32_t)
DECLARE_MEMCPY_VALUE(int64_t, int32_t)
DECLARE_MEMCPY_VALUE(uint64_t, int32_t)

/**
 * Key2 is uint32_t
 */

DECLARE_MEMCPY_VALUE(int8_t, uint32_t)
DECLARE_MEMCPY_VALUE(uint8_t, uint32_t)
DECLARE_MEMCPY_VALUE(int16_t, uint32_t)
DECLARE_MEMCPY_VALUE(uint16_t, uint32_t)
DECLARE_MEMCPY_VALUE(int32_t, uint32_t)
DECLARE_MEMCPY_VALUE(uint32_t, uint32_t)
DECLARE_MEMCPY_VALUE(int64_t, uint32_t)
DECLARE_MEMCPY_VALUE(uint64_t, uint32_t)

/**
 * Key2 is int64_t
 */

DECLARE_MEMCPY_VALUE(int8_t, int64_t)
DECLARE_MEMCPY_VALUE(uint8_t, int64_t)
DECLARE_MEMCPY_VALUE(int16_t, int64_t)
DECLARE_MEMCPY_VALUE(uint16_t, int64_t)
DECLARE_MEMCPY_VALUE(int32_t, int64_t)
DECLARE_MEMCPY_VALUE(uint32_t, int64_t)
DECLARE_MEMCPY_VALUE(int64_t, int64_t)
DECLARE_MEMCPY_VALUE(uint64_t, int64_t)

/**
 * Key2 is uint64_t
 */

DECLARE_MEMCPY_VALUE(int8_t, uint64_t)
DECLARE_MEMCPY_VALUE(uint8_t, uint64_t)
DECLARE_MEMCPY_VALUE(int16_t, uint64_t)
DECLARE_MEMCPY_VALUE(uint16_t, uint64_t)
DECLARE_MEMCPY_VALUE(int32_t, uint64_t)
DECLARE_MEMCPY_VALUE(uint32_t, uint64_t)
DECLARE_MEMCPY_VALUE(int64_t, uint64_t)
DECLARE_MEMCPY_VALUE(uint64_t, uint64_t)

#endif
