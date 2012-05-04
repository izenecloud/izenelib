#include <am/succinct/fujimap/fujimap.hpp>

NS_IZENELIB_AM_BEGIN

namespace succinct{ namespace fujimap{

template <>
uint64_t Fujimap<uint128_t>::getBlockID(const uint128_t& key) const
{
    return key % keyBlockN_;
}

}}

NS_IZENELIB_AM_END
