/* 
 * File:   MessageTest.cpp
 * Author: Paolo D'Apice
 * 
 * Created on May 29, 2012, 10:04 AM
 */

#define BOOST_TEST_MODULE MessageTest
#include <boost/test/unit_test.hpp>

#include "net/distribute/Message.hpp"
#include "util/test/BoostTestThreadSafety.h"

using namespace NS_IZENELIB_DISTRIBUTE;

#define PRINT(X) std::cout << #X << ": " << X << std::endl

BOOST_AUTO_TEST_CASE(request) {
    Request input("/path/to/file", 1024, "/dest/dir");
    PRINT(input);
    
    // pack
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, input);
    
    // unpack
    msgpack::unpacked unp;
    msgpack::unpack(&unp, sbuf.data(), sbuf.size());
    
    Request output = unp.get().as<Request>();
    PRINT(output);
    
    BOOST_CHECK_EQUAL(input.getFileName(), output.getFileName());
    BOOST_CHECK_EQUAL(input.getFileSize(), output.getFileSize());
    BOOST_CHECK_EQUAL(input.getDestination(), output.getDestination());
}

BOOST_AUTO_TEST_CASE(ack) {
    RequestAck input(true);
    PRINT(input);
    
    // pack
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, input);
    
    // unpack
    msgpack::unpacked unp;
    msgpack::unpack(&unp, sbuf.data(), sbuf.size());
    
    RequestAck output = unp.get().as<RequestAck>();
    PRINT(output);
    
    BOOST_CHECK_EQUAL(input.getStatus(), output.getStatus());
    BOOST_CHECK_EQUAL(input.getSize(), output.getSize());
}
