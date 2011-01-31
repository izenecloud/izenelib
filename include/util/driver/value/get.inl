#ifdef IZENELIB_DRIVER_VALUE_INLINE
/**
 * @file izenelib/driver/value/get.inl
 * @author Ian Yang
 * @date Created <2010-06-08 15:04:23>
 * @brief get functions for izenelib::driver::Value
 */
namespace izenelib {
namespace driver {

template<typename T>
const T& Value::get() const
{
    using boost::get;
    return get<T>(value_);
}

template<typename T>
T& Value::get()
{
    using boost::get;
    return get<T>(value_);
}

template<typename T>
T& Value::getOrReset()
{
    return *getPtrOrReset<T>();
}

template<typename T>
const T* Value::getPtr() const
{
    using boost::get;
    return get<T>(&value_);
}

template<typename T>
T* Value::getPtr()
{
    using boost::get;
    return get<T>(&value_);
}

template<typename T>
T* Value::getPtrOrReset()
{
    T* ret = getPtr<T>();
    if (!ret)
    {
        reset<T>();
        ret = getPtr<T>();
    }

    BOOST_ASSERT(ret);
    return ret;
}

/// @see Value::getPtr
template<typename T>
inline T* get(Value* value)
{
    return value->getPtr<T>();
}

/// @see Value::getPtr
template<typename T>
inline const T* get(const Value* value)
{
    return value->getPtr<T>();
}

/// @see Value::get
template<typename T>
inline T& get(Value& value)
{
    return value.get<T>();
}

template<typename T>
inline const T& get(const Value& value)
{
    return value.get<T>();
}

/// @see Value::getPtrOrReset
template<typename T>
inline T* getOrReset(Value* value)
{
    return value->getPtrOrReset<T>();
}

/// @see Value::getOrReset
template<typename T>
inline T& getOrReset(Value& value)
{
    return value.getOrReset<T>();
}

}} // namespace izenelib::driver

#endif // IZENELIB_DRIVER_VALUE_INLINE
