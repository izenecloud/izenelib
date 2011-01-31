#ifndef IZENELIB_DRIVER_VALUE_FORMATTERS_H
#define IZENELIB_DRIVER_VALUE_FORMATTERS_H
/**
 * @file izenelib/driver/value/formatters.h
 * @author Ian Yang
 * @date Created <2010-07-02 10:48:24>
 * @brief to*String functions
 */
#include <string>
namespace izenelib {
namespace driver {
class Value;

std::string toJsonString(const Value& value);

std::string toPrettyJsonString(const Value& value);

}} // namespace izenelib::driver

#endif // IZENELIB_DRIVER_VALUE_FORMATTERS_H
