#ifdef IZENELIB_DRIVER_VALUE_INLINE
/**
 * @file izenelib/driver/value/size.inl
 * @author Ian Yang
 * @date Created <2010-06-08 11:31:29>
 * @brief Gets size of a value
 */
namespace izenelib {
namespace driver {

namespace detail {

class ValueSizeVisitor : public boost::static_visitor<std::size_t>
{
public:
    typedef std::size_t size_type;

    /// @brief Returns 1 for all singular value
    template<typename T>
    size_type operator()(const T&) const
    {
        return 1;
    }

    /// @brief Returns 0 for Null
    size_type operator()(const Value::NullType&) const
    {
        return 0;
    }

    /// @brief Gets size of Array
    size_type operator()(const Value::ArrayType& value) const
    {
        return value.size();
    }

    /// @brief Gets size of Object
    size_type operator()(const Value::ObjectType& value) const
    {
        return value.size();
    }
};

} // namespace detail

std::size_t Value::size() const
{
    return boost::apply_visitor(detail::ValueSizeVisitor(), value_);
}

}} // namespace izenelib::driver

#endif // IZENELIB_DRIVER_VALUE_INLINE
