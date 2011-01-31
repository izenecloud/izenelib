/**
 * @file core/driver/value/formatters.cpp
 * @author Ian Yang
 * @date Created <2010-07-02 10:48:09>
 */
#include <util/driver/value/formatters.h>

#include <util/driver/writers/JsonWriter.h>
#include <util/driver/writers/PrettyJsonWriter.h>

namespace izenelib {
namespace driver {

std::string toJsonString(const Value& value)
{
    JsonWriter writer;
    std::string result;

    writer.write(value, result);

    return result;
}

std::string toPrettyJsonString(const Value& value)
{
    PrettyJsonWriter writer;
    std::string result;

    writer.write(value, result);

    return result;
}

}} // namespace izenelib::driver

