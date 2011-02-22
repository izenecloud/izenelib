#ifndef IZENELIB_DRIVER_KEYS_H
#define IZENELIB_DRIVER_KEYS_H
/**
 * @file izenelib/driver/Keys.h
 * @author Ian Yang
 * @date Created <2010-06-10 15:28:15>
 * @brief Key names used as convention.
 */
#include <boost/preprocessor.hpp>

#include <string>

#include "Keys.inl"

namespace izenelib {
namespace driver {

#define IZENLIB_DRIVER_KEYS_DECL(z,i,l) \
    static const std::string BOOST_PP_SEQ_ELEM(i, l);

struct Keys
{
    BOOST_PP_REPEAT(
        BOOST_PP_SEQ_SIZE(IZENELIB_DRIVER_KEYS),
        IZENLIB_DRIVER_KEYS_DECL,
        IZENELIB_DRIVER_KEYS
    )
};

#undef IZENLIB_DRIVER_KEYS_DECL
}} // namespace izenelib::driver

#endif // IZENELIB_DRIVER_KEYS_H
