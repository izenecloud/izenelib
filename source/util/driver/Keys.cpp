/**
 * @file core/driver/Keys.cpp
 * @author Ian Yang
 * @date Created <2010-06-10 15:29:55>
 */
#include <util/driver/Keys.h>


namespace izenelib {
namespace driver {

#define SF1_DRIVER_KEYS_DEF(z,i,l) \
    const std::string Keys::BOOST_PP_SEQ_ELEM(i,l) = \
        BOOST_PP_STRINGIZE(BOOST_PP_SEQ_ELEM(i,l));

BOOST_PP_REPEAT(
    BOOST_PP_SEQ_SIZE(SF1_DRIVER_KEYS),
    SF1_DRIVER_KEYS_DEF,
    SF1_DRIVER_KEYS
)

#undef SF1_DRIVER_KEYS_DEF
}} // namespace izenelib::driver
