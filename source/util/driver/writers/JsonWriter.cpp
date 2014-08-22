/**
 * @file core/driver/writers/JsonWriter.cpp
 * @author Ian Yang
 * @date Created <2010-06-09 16:40:08>
 */
#include <util/driver/writers/JsonWriter.h>

#include "DriverValue2JsonValue.h"

#include <3rdparty/rapidjson/writer.h>
#include <3rdparty/rapidjson/stringbuffer.h>
#include <json/json.h>

#include <boost/assert.hpp>

namespace rj = rapidjson;
namespace izenelib {
namespace driver {

void JsonWriter::write(const Value& value, std::string& document)
{
    //Json::Value root;
    //Json::FastWriter writer;

    //driverValue2JsonValue(value, root);
    //document = writer.write(root);

    rj::Document doc;
    rj::Value json_value;
    driverValue2JsonValue(value, json_value, doc.GetAllocator());

    rj::StringBuffer buf;
    rj::Writer<rj::StringBuffer> writer(buf);
    json_value.Accept(writer);
    document.assign(buf.GetString(), buf.Size());
}

}} // namespace izenelib::driver
