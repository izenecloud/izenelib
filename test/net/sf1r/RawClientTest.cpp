/* 
 * File:   RawClientTest.cpp
 * Author: Paolo D'Apice
 *
 * Created on January 4, 2011, 10:46 PM
 */

#define BOOST_TEST_MODULE RawClientTest
#include <boost/test/unit_test.hpp>

#include "net/sf1r/RawClient.hpp"
#include <exception>
#include <iostream>
using namespace std;


BOOST_AUTO_TEST_CASE(error_test) {
    string host = "somewhere";
    string port = "port";
    RawClient client(host, port);
    try {
        client.connect();
        BOOST_FAIL("Exception expected");
    } catch (exception& e) {
        BOOST_MESSAGE(e.what());
    } catch (...) {
        BOOST_FAIL("Unexpected exception");
    }
}


#if 0 // don't know if there is a running SF1

static const string HOST = "localhost";
static const string PORT = "18181";

BOOST_AUTO_TEST_CASE(connection_test) {
    RawClient client(HOST, PORT);
    BOOST_CHECK(client.isConnected() == false);
    
    client.connect();
    BOOST_CHECK(client.isConnected() == true);
    BOOST_CHECK_EQUAL("localhost:18181", client.serverName());
    
    client.reconnect();
    BOOST_CHECK(client.isConnected() == true);
    
    client.close();
    BOOST_CHECK(client.isConnected() == false);
}


BOOST_AUTO_TEST_CASE(send_receive_test) {
    const unsigned sequence = 1234567890;
    const string message = "{\"header\":{\"controller\":\"test\",\"action\":\"echo\"},\"message\":\"Ciao! 你好！\"}";
    const string expected = "{\"header\":{\"success\":true},\"message\":\"Ciao! 你好！\"}";
    
    RawClient client(HOST, PORT);
    
    try {
        client.sendRequest(sequence, message);
        BOOST_FAIL("Exception expected");
    } catch (exception& e) {
        BOOST_MESSAGE(e.what());
    } catch (...) {
        BOOST_FAIL("Unexpected exception");
    }
    
    try {
        client.getResponse();
        BOOST_FAIL("Exception expected");
    } catch (exception& e) {
        BOOST_MESSAGE(e.what());
    } catch (...) {
        BOOST_FAIL("Unexpected exception");
    }
    
    client.connect();
    BOOST_CHECK(client.isConnected() == true);
     
    client.sendRequest(sequence, message);
    pair<unsigned, string> response =  client.getResponse();
    BOOST_CHECK_EQUAL(sequence, response.first);
    BOOST_CHECK_EQUAL(expected, response.second);
}

#endif
