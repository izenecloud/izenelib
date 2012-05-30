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
#include "net/distribute/DataTransfer2.hpp"
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

using namespace NS_IZENELIB_DISTRIBUTE;

namespace {
const std::string BASEDIR = bfs::temp_directory_path().string();
const std::string HOST = "localhost";
const unsigned PORT = 8121;
}


BOOST_AUTO_TEST_CASE(service_test) {
    DataReceiver2 server(BASEDIR, PORT);
    server.start();
    { 
        DataTransfer2 client(HOST, PORT);
        BOOST_CHECK(client.probe());
    }
    server.stop();
}

#else

BOOST_AUTO_TEST_CASE(dummy) {
    // avoid test failure due to empty test tree
}

#endif