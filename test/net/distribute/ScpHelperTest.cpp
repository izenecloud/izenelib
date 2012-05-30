/* 
 * File:   ScpHelperTest.cpp
 * Author: Paolo D'Apice
 * 
 * Created on May 30, 2012, 01:15 PM
 */

#define BOOST_TEST_MODULE ScpHelperTest
#include <boost/test/unit_test.hpp>

#ifdef ENABLE_TEST

/**
 * Fix the argument of the ScpHelper::scp function properly.
 */
#include "net/distribute/ScpHelper.hpp"

using NS_IZENELIB_DISTRIBUTE::ScpHelper;

BOOST_AUTO_TEST_CASE(scp) {
    int ret = ScpHelper::scp("tmp/testfile", "localhost:/tmp/copied");    
    BOOST_CHECK(ret == 0);
}

#else

BOOST_AUTO_TEST_CASE(dummy) {
    // avoid test failure due to empty test tree
}

#endif