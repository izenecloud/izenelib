#ifndef IZENELIB_DRIVER_VALUE_TYPES_H
#define IZENELIB_DRIVER_VALUE_TYPES_H
/**
 * @file izenelib/driver/value/types.h
 * @author Ian Yang
 * @date Created <2010-06-10 10:49:58>
 */

#include <types.h> // int64_t, uint64_t

#include <string>
#include <vector>
#include <map>

namespace izenelib {
namespace driver {

class Value;

/// @brief tags for null type
class NullTypeTag {};
inline bool operator==(NullTypeTag, NullTypeTag)
{
    return true;
}
inline bool operator<(NullTypeTag, NullTypeTag)
{
    return false;
}

/// Require this, because comma is separator in macro
typedef std::map<std::string, Value> NamedValueMapType;

typedef bool (*NullValueFunc)(const Value&);

/// @brief Type list. Null must be the first.
#define IZENELIB_DRIVER_VALUE_TYPES \
    ((NullTypeTag, Null)) \
    ((int64_t, Int)) \
    ((uint64_t, Uint)) \
    ((double, Double)) \
    ((std::string, String)) \
    ((bool, Bool)) \
    ((std::vector<Value>, Array)) \
    ((NamedValueMapType, Object))

}} // namespace izenelib::driver

#endif // IZENELIB_DRIVER_VALUE_TYPES_H
