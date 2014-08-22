/**
 * @file core/driver/writers/JsonWriter.cpp
 * @author Ian Yang
 * @date Created <2010-06-09 16:40:08>
 */
#include <util/driver/writers/JsonWriter.h>

#include "DriverValue2JsonValue.h"

#include <json/json.h>

#include <boost/assert.hpp>

namespace izenelib {
namespace driver {

void JsonWriter::write(const Value& value, std::string& document)
{
    Json::Value root;
    Json::FastWriter writer;

    driverValue2JsonValue(value, root);
    document = writer.write(root);
}

}} // namespace izenelib::driver
