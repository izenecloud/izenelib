/* 
 * File:   Sf1DriverTest.cpp
 * Author: Paolo D'Apice
 * 
 * Created on January 9, 2012, 10:22 AM
 */

#define BOOST_TEST_MODULE Sf1DriverTest
#include <boost/test/unit_test.hpp>

#include "net/sf1r/Sf1Driver.hpp"
#include <string>

using namespace izenelib::net::sf1r;
using namespace std;


BOOST_AUTO_TEST_CASE(error_test) {
    string host = "somewhere";
    
    try {
        Sf1Driver driver(host);
        BOOST_FAIL("ServerError expected");
    } catch(ServerError& e) {
    } catch(...) {
        BOOST_FAIL("Unexpected exception");
    }
}


#if 0 // don't know if there is a running SF1

static const string HOST = "localhost";
static const int PORT = 18181;


BOOST_AUTO_TEST_CASE(echo_test) {
    const string requestUri    = "/test/echo"; 
    const string requestTokens = "token";
          string requestBody   = "{\"message\":\"Ciao! 你好！\"}";
    const string expected      = "{\"header\":{\"success\":true},\"message\":\"Ciao! 你好！\"}";

    Sf1Driver driver(HOST, PORT);
    BOOST_CHECK_EQUAL(1, driver.getSequence());
    
    string response = driver.call(requestUri, requestTokens, requestBody);
    BOOST_CHECK_EQUAL(2, driver.getSequence());
    BOOST_CHECK_EQUAL(expected, response);
    BOOST_CHECK(driver.getSequence() != 0);
}


BOOST_AUTO_TEST_CASE(search_test) {
    const string requestUri    = "/documents/search"; 
    const string requestTokens = "";
          string requestBody   = "{\"collection\":\"example\","
                                  "\"header\":{"
                                      "\"check_time\":true"
                                  "},"
                                  "\"search\":{\"keywords\":\"america\"},"
                                  "\"limit\":10}";
    
    Sf1Driver driver(HOST, PORT);
    string response = driver.call(requestUri, requestTokens, requestBody);
    BOOST_CHECK_EQUAL(2, driver.getSequence());
    //BOOST_CHECK_EQUAL(expected, response);
    BOOST_CHECK(driver.getSequence() != 0);
}


BOOST_AUTO_TEST_CASE(badRequest_test) {
    vector<string> requests;
    requests.push_back("/bad/request");
    requests.push_back("/test/bad");
    requests.push_back("/test");
    
    string tokens = "";
    const string expected = "{\"errors\":[\"Handler not found\"],\"header\":{\"success\":false}}";

    Sf1Driver driver(HOST, PORT);
    
    unsigned seq = 1;
    for (vector<string>::iterator it = requests.begin(); it < requests.end(); it++) {
        BOOST_CHECK_EQUAL(seq++, driver.getSequence());
        string request = "{\"message\":\"Ciao! 你好！\"}";
        string response = driver.call(*it, tokens, request);
        BOOST_CHECK_EQUAL(expected, response);
        BOOST_CHECK(driver.getSequence() != 0);
    }
}


BOOST_AUTO_TEST_CASE(ServerError_test) {
    vector<string> requests;
    requests.push_back("/");
    
    string tokens = "";
    
    Sf1Driver driver(HOST, PORT);
    
    unsigned seq = 1;
    for (vector<string>::iterator it = requests.begin(); it < requests.end(); it++) {
        BOOST_CHECK_EQUAL(seq++, driver.getSequence());
        string request = "{\"message\":\"Ciao! 你好！\"}";
        try {
            driver.call(*it, tokens, request);
            BOOST_FAIL("ServerError expected");
        } catch (ServerError& e) {
        } catch (...) {
            BOOST_FAIL("Unexpected exception");
        }
    }
}

#endif
