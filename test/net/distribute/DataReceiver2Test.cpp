/* 
 * File:   DataTransfer2Test.cpp
 * Author: Paolo D'Apice
 * 
 * Created on May 22, 2012, 2:34 PM
 */

#define BOOST_TEST_MODULE DataReceiver2
#include <boost/test/unit_test.hpp>

#ifdef ENABLE_TEST

#include "net/distribute/DataReceiver2.hpp"

using namespace NS_IZENELIB_DISTRIBUTE;
using namespace std;

namespace {
const unsigned PORT = 8121;
}


BOOST_AUTO_TEST_CASE(main_t) {
    DataReceiver2 server(PORT);
    server.start();
    cout << "Press ENTER to stop" << endl;
    cin.get();
    server.stop();
}

#else

BOOST_AUTO_TEST_CASE(dummy) {
    // avoid test failure due to empty test tree
}

#endif