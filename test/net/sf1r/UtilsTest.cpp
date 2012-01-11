/* 
 * File:   UtilsTest.cpp
 * Author: Paolo D'Apice
 *
 * Created on January 5, 2011, 12:07 PM
 */

#define BOOST_TEST_MODULE UtilsTest
#include <boost/test/unit_test.hpp>

#include "net/sf1r/Utils.hpp"


BOOST_AUTO_TEST_CASE(headerSize_test) {
    BOOST_CHECK_EQUAL(4, sizeof(unsigned));
    BOOST_CHECK_EQUAL(8, headerSize());
}
