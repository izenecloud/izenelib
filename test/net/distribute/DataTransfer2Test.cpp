/* 
 * File:   DataTransfer2Test.cpp
 * Author: Paolo D'Apice
 * 
 * Created on May 22, 2012, 2:34 PM
 */

#define BOOST_TEST_MODULE DataTransfer2
#include <boost/test/unit_test.hpp>

#include "net/distribute/DataTransfer2.hpp"
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

using namespace izenelib::net::distribute;
using namespace std;

namespace {
const string   HOST = "localhost";
const unsigned PORT = 18121;
}

#ifdef ENABLE_TEST

BOOST_AUTO_TEST_CASE(connection_fail) {
    DataTransfer2 transfer("some_host", PORT);
    BOOST_CHECK_EQUAL(false, transfer.syncSend("testfile", "tmp"));
}


BOOST_AUTO_TEST_CASE(file_error) {
    DataTransfer2 transfer(HOST, PORT);
    BOOST_CHECK_EQUAL(false, transfer.syncSend("/path/to/somefile", "tmp"));
}


BOOST_AUTO_TEST_CASE(send_file) {
    DataTransfer2 transfer(HOST, PORT);
    
    BOOST_CHECK(transfer.syncSend("testfile"));
    BOOST_CHECK(transfer.syncSend("testfile", "dest/"));
    BOOST_CHECK(transfer.syncSend("tmp/testfile", "dest"));
}


BOOST_AUTO_TEST_CASE(send_dir) {
    DataTransfer2 transfer(HOST, PORT);
    
    BOOST_CHECK(transfer.syncSend("tmp"));
    BOOST_CHECK(transfer.syncSend("tmp", "dest/"));
    BOOST_CHECK(transfer.syncSend("tmp", "dest", true));
}

#else

BOOST_AUTO_TEST_CASE(dummy) {
    // avoid test failure due to empty test tree
}

#endif