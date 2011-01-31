/**
 * @file core/driver/writers/DriverValue2JsonValue.cpp
 * @author Ian Yang
 * @date Created <2010-07-02 10:43:25>
 */
#include "DriverValue2JsonValue.h"

namespace izenelib {
namespace driver {

void driverValue2JsonValue(const Value& value, Json::Value& json)
{
    const Value::ArrayType* array = 0;
    const Value::ObjectType* object = 0;

    switch (value.type())
    {
    case Value::kIntType:
        json = static_cast<Json::Value::Int>(value.getInt());
        break;
    case Value::kUintType:
        json = static_cast<Json::Value::UInt>(value.getUint());
        break;
    case Value::kDoubleType:
        json = value.getDouble();
        break;
    case Value::kStringType:
        json = value.getString();
        break;
    case Value::kBoolType :
        json = value.getBool();
        break;
    case Value::kArrayType :
        json = Json::arrayValue;
        array = value.getArrayPtr();
        json.resize(array->size());
        for (std::size_t i = 0; i < array->size(); ++i)
        {
            driverValue2JsonValue((*array)[i], json[i]);
        }
        break;
    case Value::kObjectType:
        json = Json::objectValue;
        object = value.getObjectPtr();
        for (Value::ObjectType::const_iterator it = object->begin(),
                                            itEnd = object->end();
             it != itEnd; ++it)
        {
            if (!nullValue(it->second))
            {
                // skip NULL
                driverValue2JsonValue(it->second, json[it->first]);
            }
        }
        break;
    case Value::kNullType:
        // fall through
    default:
        json = Json::Value();
        break;
    }
}

}}
