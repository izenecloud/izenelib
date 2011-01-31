#ifndef IZENELIB_DRIVER_VALUE_DEF_H
#define IZENELIB_DRIVER_VALUE_DEF_H
/**
 * @file izenelib/driver/value/def.h
 * @author Ian Yang
 * @date Created <2010-06-10 09:56:22>
 * @brief Default value for various type.
 */
#include <boost/utility/value_init.hpp>

namespace izenelib {
namespace driver {

namespace detail {
template<typename T>
struct DefWrapper
{
    static inline const T& value()
    {
        return value_.data();
    }

private:
    static const boost::value_initialized<T> value_;
};

template<typename T>
const boost::value_initialized<T> DefWrapper<T>::value_;

} // namespace detail

template<typename T>
inline const T& def()
{
    return detail::DefWrapper<T>::value();
}

}} // namespace izenelib::driver

#endif // IZENELIB_DRIVER_VALUE_DEF_H
