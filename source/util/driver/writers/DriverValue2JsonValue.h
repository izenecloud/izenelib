#ifndef CORE_DRIVER_WRITERS_DRIVER_VALUE2JSON_VALUE_H
#define CORE_DRIVER_WRITERS_DRIVER_VALUE2JSON_VALUE_H
/**
 * @file core/driver/writers/DriverValue2JsonValue.h
 * @author Ian Yang
 * @date Created <2010-07-02 10:43:25>
 */
#include <util/driver/Value.h>
#include <json/json.h>
#include <3rdparty/rapidjson/writer.h>
#include <3rdparty/rapidjson/document.h>

namespace izenelib {
namespace driver {

void driverValue2JsonValue(const Value& value, Json::Value& json);
void driverValue2JsonValue(const Value& value, rapidjson::Value& json, rapidjson::Document::AllocatorType& alloc);

}}

#endif // CORE_DRIVER_WRITERS_DRIVER_VALUE2JSON_VALUE_H
