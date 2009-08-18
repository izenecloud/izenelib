#ifndef _TRAITS_H_
#define _TRAITS_H_

#include <limits.h>

NS_IZENELIB_AM_BEGIN

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

NS_IZENELIB_AM_END

#endif

