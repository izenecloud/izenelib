/**
 * @file core/driver/RestrictedObjectValue.cpp
 * @author Ian Yang
 * @date Created <2010-06-10 15:42:25>
 */
#include <util/driver/RestrictedObjectValue.h>

namespace izenelib {
namespace driver {

RestrictedObjectValue::RestrictedObjectValue()
: value_(def<Value::ObjectType>()), top_(0)
{
    updateTopPtr();
}

RestrictedObjectValue::RestrictedObjectValue(const RestrictedObjectValue& other)
: value_(other.value_), top_(0)
{
    updateTopPtr();
}

RestrictedObjectValue& RestrictedObjectValue::operator=(
    const RestrictedObjectValue& other
)
{
    value_ = other.value_;
    updateTopPtr();

    return *this;
}

const Value& RestrictedObjectValue::operator[](const std::string& key) const
{
    Value::ObjectType::const_iterator it = top_->find(key);
    if (it != top_->end())
    {
        return it->second;
    }
    return def<Value>();
}

void RestrictedObjectValue::assignTmp(Value& value)
{
    value_.swap(value);
    updateTopPtr();
}

}} // namespace izenelib::driver
