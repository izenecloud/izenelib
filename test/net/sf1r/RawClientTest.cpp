/* 
 * File:   RawClientTest.cpp
 * Author: Paolo D'Apice
 *
 * Created on January 4, 2011, 10:46 PM
 */

#define BOOST_TEST_MODULE RawClientTest
#include <boost/test/unit_test.hpp>

#include "net/sf1r/RawClient.hpp"
#include <boost/asio.hpp>
#include <exception>
#include <iostream>

using namespace izenelib::net::sf1r;
using ba::ip::tcp;
using namespace std;


/** Check number of bytes used in the header. */
BOOST_AUTO_TEST_CASE(headerSize_test) {
    BOOST_CHECK_EQUAL(4, sizeof(uint32_t));
}


#ifdef ENABLE_SF1_TEST // don't know if there is a running SF1

/** Test fixture. */
struct AsioService {
    AsioService() : host("localhost"), port("18181"), resolver(service) {}
    ~AsioService() {}
    
    tcp::resolver::iterator
    resolve() {
        tcp::resolver::query query(host, port);
        return resolver.resolve(query);
    }
    
    const string host;
    const string port;
    ba::io_service service;
    tcp::resolver resolver;
    
};


BOOST_FIXTURE_TEST_CASE(connection_test, AsioService) {
    tcp::resolver::iterator endpoint = resolve();
    
    RawClient client(service, endpoint);
    BOOST_CHECK(client.isConnected());
}


BOOST_FIXTURE_TEST_CASE(send_receive_test, AsioService) {
    const uint32_t sequence = 1234567890;
    const string    message = "{\"header\":{\"controller\":\"test\",\"action\":\"echo\"},\"message\":\"Ciao! 你好！\"}";
    const string   expected = "{\"header\":{\"success\":true},\"message\":\"Ciao! 你好！\"}";
    
    tcp::resolver::iterator endpoint = resolve();
    RawClient client(service, endpoint);
    BOOST_CHECK(client.isConnected());
    
    client.sendRequest(sequence, message);
    pair<uint32_t, string> response =  client.getResponse();
    BOOST_CHECK_EQUAL(sequence, response.first);
    BOOST_CHECK_EQUAL(expected, response.second);
}

#endif
