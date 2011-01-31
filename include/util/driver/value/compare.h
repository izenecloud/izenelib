#ifndef IZENELIB_DRIVER_VALUE_COMPARE_H
#define IZENELIB_DRIVER_VALUE_COMPARE_H
/**
 * @file izenelib/driver/value/compare.h
 * @author Ian Yang
 * @date Created <2010-06-08 16:10:21>
 */
#include "Value.h"

namespace izenelib {
namespace driver {

inline bool operator==(const Value& a, const Value& b)
{
    return (nullValue(a) && nullValue(b)) ||
        a.variant() == b.variant();
}
inline bool operator<(const Value& a, const Value& b)
{
    if (nullValue(a) && nullValue(b))
    {
        return false;
    }
    return a.variant() < b.variant();
}

inline bool operator!=(const Value& a, const Value& b)
{
    return !(a == b);
}

inline bool operator>(const Value& a, const Value& b)
{
    return b < a;
}

inline bool operator<=(const Value& a, const Value& b)
{
    return a < b || a == b;
}

inline bool operator>=(const Value& a, const Value& b)
{
    return !(a < b);
}

}} // namespace izenelib::driver

#endif // IZENELIB_DRIVER_VALUE_COMPARE_H
