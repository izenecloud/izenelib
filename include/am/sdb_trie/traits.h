#ifndef _TRAITS_H_
#define _TRAITS_H_

#include <limits.h>

NS_IZENELIB_AM_BEGIN


/**
 * Traits for NodeID
 */
template<typename NIDType> class NodeIDTraits;

template <>
class NodeIDTraits<int32_t>
{
public:
  enum{MinValue = 1, MaxValue = INT_MAX, RootValue = 0};
};

template <>
class NodeIDTraits<uint32_t>
{
public:
  enum{MinValue = 1, MaxValue = UINT_MAX, RootValue = 0};
};

template <>
class NodeIDTraits<int64_t>
{
public:
  enum{MinValue = 1, MaxValue = LONG_MAX, RootValue = 0};
};

template <>
class NodeIDTraits<uint64_t>
{
public:
  enum{MinValue = 1, MaxValue = ULONG_MAX, RootValue = 0};
};

/**
 * Traits for CharType
 */
template<typename NumberType> class NumericTraits;

template <>
class NumericTraits<char>
{
public:
  enum{MinValue = SCHAR_MIN, MaxValue = SCHAR_MAX};
};

template <>
class NumericTraits<int8_t>
{
public:
  enum{MinValue = SCHAR_MIN, MaxValue = SCHAR_MAX};
};

template <>
class NumericTraits<uint8_t>
{
public:
  enum{MinValue = 0, MaxValue = UCHAR_MAX};
};

template <>
class NumericTraits<int16_t>
{
public:
  enum{MinValue = SHRT_MIN, MaxValue = SHRT_MAX};
};

template <>
class NumericTraits<uint16_t>
{
public:
  enum{MinValue = 0, MaxValue = USHRT_MAX};
};

template <>
class NumericTraits<int32_t>
{
public:
  enum{MinValue = INT_MIN, MaxValue = INT_MAX};
};

template <>
class NumericTraits<uint32_t>
{
public:
  enum{MinValue = 0, MaxValue = UINT_MAX};
};

template <>
class NumericTraits<int64_t>
{
public:
  enum{MinValue = LONG_MIN, MaxValue = LONG_MAX};
};

template <>
class NumericTraits<uint64_t>
{
public:
  enum{MinValue = 0, MaxValue = ULONG_MAX};
};

NS_IZENELIB_AM_END

#endif

