#ifdef IZENELIB_DRIVER_VALUE_INLINE
/**
 * @file izenelib/driver/value/clear.inl
 * @author Ian Yang
 * @date Created <2010-06-08 11:31:29>
 * @brief Clear collection
 */
namespace izenelib {
namespace driver {

namespace detail {

class ValueClearVisitor : public boost::static_visitor<>
{
public:
    template<typename T>
    void operator()(T& value) const
    {
        value = def<T>();
    }

    void operator()(Value::NullType&) const
    {}

    /// @brief Clear string
    void operator()(Value::StringType& value) const
    {
        value.resize(0);
    }

    /// @brief Clear array
    void operator()(Value::ArrayType& value) const
    {
        value.resize(0);
    }

    /// @brief Clear object
    void operator()(Value::ObjectType& value) const
    {
        value.clear();
    }
};

} // namespace detail

void Value::clear()
{
    boost::apply_visitor(detail::ValueClearVisitor(), value_);
}

}} // namespace izenelib::driver

#endif // IZENELIB_DRIVER_VALUE_INLINE
