#ifndef IZENELIB_DRIVER_VALUE_NULL_VALUE_H
#define IZENELIB_DRIVER_VALUE_NULL_VALUE_H
/**
 * @file izenelib/driver/value/NullValue.h
 * @author Ian Yang
 * @date Created <2010-06-07 18:06:17>
 * @brief Forward declaration of nullValue
 */

#include "Value.h"

namespace izenelib {
namespace driver {

/// @brief Keyword to represent null type. It is also can be used to test
/// weather a value is of null type.
/// @return Returns \c true if \a value is of null type
inline bool nullValue(const Value& value)
{
    return value.type() == Value::kNullType;
}

}} // namespace izenelib::driver

#endif // IZENELIB_DRIVER_VALUE_NULL_VALUE_H
