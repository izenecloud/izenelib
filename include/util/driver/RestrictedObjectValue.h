#ifndef IZENELIB_DRIVER_RESTRICTED_OBJECT_VALUE_H
#define IZENELIB_DRIVER_RESTRICTED_OBJECT_VALUE_H
/**
 * @file izenelib/driver/RestrictedObjectValue.h
 * @author Ian Yang
 * @date Created <2010-06-10 15:40:50>
 * @brief A wrapper of Value restricting it to be a object value.
 */
#include "Value.h"

namespace izenelib {
namespace driver {

/// @brief A wrapper of Value restricting it to be a object value.
class RestrictedObjectValue
{
public:
    RestrictedObjectValue();
    RestrictedObjectValue(const RestrictedObjectValue& other);
    RestrictedObjectValue& operator=(const RestrictedObjectValue& other);

    Value& operator[](const std::string& key)
    {
        return (*top_)[key];
    }

    const Value& operator[](const std::string& key) const;

    void swap(RestrictedObjectValue& other)
    {
        using std::swap;
        swap(value_, other.value_);
        updateTopPtr();
        other.updateTopPtr();
    }

    void assignTmp(Value& value);

    const Value& get() const
    {
        return value_;
    }

private:
    void updateTopPtr()
    {
        top_ = value_.getPtr<Value::ObjectType>();
    }

    Value value_;

    /// @brief shortcuts to top object value
    Value::ObjectType* top_;
};

}} // namespace izenelib::driver

#endif // IZENELIB_DRIVER_RESTRICTED_OBJECT_VALUE_H
