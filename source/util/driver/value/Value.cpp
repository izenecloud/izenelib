/**
 * @file core/driver/value/Value.cpp
 * @author Ian Yang
 * @date Created <2010-06-08 17:08:39>
 */
#include <util/driver/value/Value.h>

namespace izenelib {
namespace driver {

Value::Value()
: value_()
{}

Value::Value(NullValueFunc)
  : value_()
{}

Value::Value(const Value& other)
: value_(other.value_)
{}

/// @brief Constructs a string
Value::Value(const char* str)
: value_(def<StringType>())
{
    using boost::get;
    get<StringType>(value_) = str;
}

Value::Value(bool boolValue)
: value_(BoolType(boolValue))
{}

void Value::assign(const char* str)
{
    value_ = def<StringType>();
    using boost::get;
    get<StringType>(value_) = str;
}

const Value& Value::operator[](const std::string& key) const
{
    typedef ObjectType::const_iterator iterator;

    const ObjectType* obj = getPtr<ObjectType>();
    if (obj)
    {
        iterator it = obj->find(key);
        if (it != obj->end())
        {
            return it->second;
        }
    }

    return def<Value>();
}

bool Value::hasKey(const std::string key) const
{
    if (type() == kObjectType)
    {
        const ObjectType& obj = get<ObjectType>();
        ObjectType::const_iterator it = obj.find(key);
        return it != obj.end() && it->second.type() != kNullType;
    }
    return false;
}

Value& Value::operator()(std::size_t index)
{
    ArrayType& arr = getOrReset<ArrayType>();
    if (index >= arr.size())
    {
        arr.resize(index + 1);
    }

    return arr[index];
}

const Value& Value::operator()(std::size_t index) const
{
    const ArrayType* arr = getPtr<ArrayType>();
    if (arr && index < arr->size())
    {
        return (*arr)[index];
    }

    return def<Value>();
}

Value& Value::operator()()
{
    ArrayType& arr = getOrReset<ArrayType>();
    arr.push_back(def<Value>());
    return arr.back();
}

}} // namespace izenelib::driver

