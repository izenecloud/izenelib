/**
 * @file core/driver/readers/JsonReader.cpp
 * @author Ian Yang
 * @date Created <2010-06-09 16:18:35>
 */
#include <util/driver/readers/JsonReader.h>

#include <json/json.h>

namespace izenelib {
namespace driver {

void jsonValue2DriverValue(const Json::Value& json, Value& value);

bool JsonReader::read(const std::string& document, Value& value)
{
    error().resize(0);

    Json::Value root;
    Json::Reader reader;

    bool parsingSuccessful = reader.parse(document, root);
    if ( !parsingSuccessful )
    {
        error() = reader.getFormatedErrorMessages();
        return false;
    }

    // Copy root to value
    // TODO: Write a reader ourselves to avoid one data copy. Json reader is
    // very easy.
    jsonValue2DriverValue(root, value);

    return true;
}

void jsonValue2DriverValue(const Json::Value& json, Value& value)
{
    Value::ArrayType* array = 0;
    Value::ObjectType* object = 0;

    switch (json.type())
    {
    case Json::intValue:
        value = json.asInt();
        break;
    case Json::uintValue:
        value = json.asUInt();
        break;
    case Json::realValue:
        value = json.asDouble();
        break;
    case Json::stringValue:
        value = json.asCString();
        break;
    case Json::booleanValue:
        value = json.asBool();
        break;
    case Json::arrayValue:
        array = value.getPtrOrReset<Value::ArrayType>();
        array->resize(json.size());
        for (std::size_t i = 0; i < json.size(); ++i)
        {
            jsonValue2DriverValue(json[i], (*array)[i]);
        }
        break;
    case Json::objectValue:
        object = value.getPtrOrReset<Value::ObjectType>();
        for (Json::Value::const_iterator it = json.begin(),
                                      itEnd = json.end();
             it != itEnd; ++it)
        {
            // skip NULL
            if (Json::nullValue != (*it).type())
            {
                jsonValue2DriverValue(*it, (*object)[it.key().asCString()]);
            }
        }
        break;
    case Json::nullValue:
        // fall through
    default:
        value = nullValue;
        break;
    }
}

}} // namespace izenelib::driver
