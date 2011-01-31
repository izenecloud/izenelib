#ifndef IZENELIB_DRIVER_VALUE_AS_H
#define IZENELIB_DRIVER_VALUE_AS_H
/**
 * @file izenelib/driver/value/as.h
 * @author Ian Yang
 * @date Created <2010-06-08 17:33:13>
 */
#include <boost/lexical_cast.hpp>

#include "def.h"
#include "macros.h"

namespace izenelib {
namespace driver {

namespace detail {

template<typename T>
class NumberConverter : public boost::static_visitor<T>
{
public:
    T operator()(Value::IntType value) const
    {
        if (boost::is_unsigned<T>::value && value < 0)
        {
            return static_cast<T>(0);
        }
        else
        {
            return static_cast<T>(value);
        }
    }
    T operator()(Value::UintType value) const
    {
        return static_cast<T>(value);
    }
    T operator()(Value::DoubleType value) const
    {
        return static_cast<T>(value);
    }
    T operator()(Value::BoolType value) const
    {
        return static_cast<T>(value ? 1 : 0);
    }
    T operator()(const Value::StringType& value) const
    {
        try
        {
            return boost::lexical_cast<T>(value);
        }
        catch (const boost::bad_lexical_cast&)
        {
            return static_cast<T>(0);
        }
    }
    template<typename V>
    T operator()(const V& value) const
    {
        return static_cast<T>(0);
    }
};

class BoolConverter : public boost::static_visitor<bool>
{
public:
    bool operator()(const Value::StringType& value) const
    {
        return !value.empty();
    }
    bool operator()(const Value::ArrayType& value) const
    {
        return !value.empty();
    }
    bool operator()(const Value::ObjectType& value) const
    {
        return !value.empty();
    }
    bool operator()(const Value::NullType& value) const
    {
        return false;
    }
    template<typename V>
    bool operator()(const V& value) const
    {
        return value;
    }
};

class StringConverter : public boost::static_visitor<Value::StringType>
{
public:
    Value::StringType operator()(Value::IntType value) const
    {
        return boost::lexical_cast<Value::StringType>(value);
    }
    Value::StringType operator()(Value::UintType value) const
    {
        return boost::lexical_cast<Value::StringType>(value);
    }
    Value::StringType operator()(Value::DoubleType value) const
    {
        return boost::lexical_cast<Value::StringType>(value);
    }
    Value::StringType operator()(Value::BoolType value) const
    {
        return Value::StringType(value ? "1" : "0");
    }
    Value::StringType operator()(const Value::StringType& value) const
    {
        return value;
    }
    template<typename V>
    Value::StringType operator()(const V& value) const
    {
        return def<Value::StringType>();
    }
};

template<typename T>
class OnlySelfConverter : public boost::static_visitor<T>
{
public:
    T operator()(const T& value) const
    {
        return value;
    }

    template<typename V>
    T operator()(const V& value) const
    {
        return def<T>();
    }
};

} // namespace detail

inline Value::IntType asInt(const Value& value)
{
    return boost::apply_visitor(detail::NumberConverter<Value::IntType>(),
                                value.variant());
}

inline Value::UintType asUint(const Value& value)
{
    return boost::apply_visitor(detail::NumberConverter<Value::UintType>(),
                                value.variant());
}

inline Value::DoubleType asDouble(const Value& value)
{
    return boost::apply_visitor(detail::NumberConverter<Value::DoubleType>(),
                                value.variant());
}

inline Value::StringType asString(const Value& value)
{
    return boost::apply_visitor(detail::StringConverter(), value.variant());
}

inline Value::BoolType asBool(const Value& value)
{
    return boost::apply_visitor(detail::BoolConverter(), value.variant());
}

IZENELIB_DRIVER_VALUE_AS_FUNCTIONS((Int)(Uint)(Double)(String)(Bool))
IZENELIB_DRIVER_VALUE_AS_OR_FUNCTIONS((Int)(Uint)(Double)(String)(Bool))

template<typename T>
inline T as(const Value& value)
{
    return as(value, def<T>());
}


/// @brief Convert value to type T if it is not NULL, otherwise returns default
/// value \a def.
template<typename T>
inline T asOr(const Value& value, const T& defValue)
{
    if (!nullValue(value))
    {
        return as(value, def<T>());
    }
    else
    {
        return defValue;
    }
}

}} // namespace izenelib::driver

#endif // IZENELIB_DRIVER_VALUE_AS_H
