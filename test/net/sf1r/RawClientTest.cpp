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
    AsioService() : host("localhost"), port("18181"), 
                    resolver(service), query(host, port) {
        iterator = resolver.resolve(query);
    }
    ~AsioService() {}
    
    const string host;
    const string port;
    
    ba::io_service service;
    tcp::resolver resolver;
    tcp::resolver::query query;
    tcp::resolver::iterator iterator;
};


BOOST_FIXTURE_TEST_CASE(connection_test, AsioService) {
    RawClient client(service, iterator);
    BOOST_CHECK(client.isConnected());
    BOOST_CHECK(client.idle());
    BOOST_CHECK_EQUAL(RawClient::Idle, client.getStatus());
}


BOOST_FIXTURE_TEST_CASE(send_receive_test, AsioService) {
    const uint32_t sequence = 1234567890;
    const string    message = "{\"header\":{\"controller\":\"test\",\"action\":\"echo\"},\"message\":\"Ciao! 你好！\"}";
    const string   expected = "{\"header\":{\"success\":true},\"message\":\"Ciao! 你好！\"}";
    
    RawClient client(service, iterator);
    BOOST_CHECK(client.isConnected());
    BOOST_CHECK(client.idle());
    
    client.sendRequest(sequence, message);
    BOOST_CHECK(not client.idle());
    BOOST_CHECK_EQUAL(RawClient::Busy, client.getStatus());
    
    Response response =  client.getResponse();
    BOOST_CHECK(client.idle());
    
    BOOST_CHECK_EQUAL(sequence, response.get<RESPONSE_SEQUENCE>());
    BOOST_CHECK_EQUAL(expected, response.get<RESPONSE_BODY>());
}

#endif
