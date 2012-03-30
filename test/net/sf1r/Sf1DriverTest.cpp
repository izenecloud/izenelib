/* 
 * File:   Sf1DriverTest.cpp
 * Author: Paolo D'Apice
 * 
 * Created on January 9, 2012, 10:22 AM
 */

#define BOOST_TEST_MODULE Sf1DriverTest
#include <boost/test/unit_test.hpp>

#include "common.h"
#include "net/sf1r/Sf1Driver.hpp"
#include <string>

using namespace NS_IZENELIB_SF1R;
using namespace std;


BOOST_AUTO_TEST_CASE(connection_fail) {
    const Sf1Config conf;
    
    BOOST_CHECK_THROW(new Sf1Driver("somewhere", conf), ServerError);
    BOOST_CHECK_THROW(new Sf1Driver("somewhere:18181", conf), ServerError);
    BOOST_CHECK_THROW(new Sf1Driver("somewhere:service", conf), ServerError);
}


/*
 * This test requires a running SF1 instance.
 */

#ifdef ENABLE_SF1_TEST


const string HOST = "localhost:18181";
const Sf1Config CONF;


BOOST_AUTO_TEST_CASE(malformed_request_uri) {
    vector<string> uris;
    uris.push_back("");
    uris.push_back("/");
    
    const string tokens = "";
    string request = "{\"message\":\"Ciao! 你好！\"}";
    
    Sf1Driver driver(HOST, CONF);
    
    for (vector<string>::iterator it = uris.begin(); it < uris.end(); ++it) {
        BOOST_CHECK_EQUAL(1, driver.getSequence()); // request not sent to SF1
        BOOST_CHECK_THROW(driver.call(*it, tokens, request), ClientError);
    }
}


BOOST_AUTO_TEST_CASE(malformed_request_body) {
    vector<string> bodies;
    bodies.push_back("{message=\"ciao\"}");
    bodies.push_back("{\"message\":\"Ciao! 你好！\"}trailing");
    bodies.push_back("trailing{\"message\":\"Ciao! 你好！\"}");
    
    const string uri    = "/test/echo"; 
    const string tokens = "token";
          
    Sf1Driver driver(HOST, CONF);
    
    for (vector<string>::iterator it = bodies.begin(); it < bodies.end(); ++it) {
        BOOST_CHECK_EQUAL(1, driver.getSequence()); // request not sent to SF1
        BOOST_CHECK_THROW(driver.call(uri, tokens, *it), ClientError);
    }
}


BOOST_AUTO_TEST_CASE(bad_uri) {
    vector<string> uris;
    uris.push_back("/bad/request");
    uris.push_back("/test/bad");
    uris.push_back("test/bad");
    uris.push_back("/test");
    uris.push_back("test");
    
    const string tokens = "";
    const string expected = "{\"errors\":[\"Handler not found\"],\"header\":{\"success\":false}}";

    Sf1Driver driver(HOST, CONF);
    
    uint32_t seq = 1;
    BOOST_CHECK_EQUAL(seq, driver.getSequence());
    
    for (vector<string>::iterator it = uris.begin(); it < uris.end(); ++it) {
        string request = "{\"message\":\"Ciao! 你好！\"}";
    
        BOOST_CHECK_EQUAL(seq++, driver.getSequence()); // request sent to SF1
        string response = driver.call(*it, tokens, request);
        BOOST_CHECK_EQUAL(expected, response);
    }
}


BOOST_AUTO_TEST_CASE(test_sequence) {
    const string uri    = "test/echo";
    const string tokens = "";
          string body   = "{\"message\":\"Ciao! 你好！\"}";
    
    Sf1Driver driver(HOST, CONF);

    uint32_t seq = 1;
    BOOST_CHECK_EQUAL(seq, driver.getSequence());
    
    const int N = 10;

    // good requests: increment sequence
    for (int i = 0; i < N; ++i) {
        uint32_t prev = driver.getSequence();
        BOOST_CHECK_NO_THROW(driver.call(uri, tokens, body));
        BOOST_CHECK_EQUAL(prev + 1, driver.getSequence());
    }
    
    BOOST_CHECK_EQUAL(N+1, driver.getSequence());
    
    // bad requests: do not increment sequencer
    BOOST_CHECK_THROW(driver.call("", tokens, body), ClientError);
    BOOST_CHECK_EQUAL(N+1, driver.getSequence());
    
    string bad = "{bad}";
    BOOST_CHECK_THROW(driver.call(uri, tokens, bad), ClientError);
    BOOST_CHECK_EQUAL(N+1, driver.getSequence());
}


BOOST_AUTO_TEST_CASE(test_echo) {
    const string uri    = "test/echo"; // works without leading '/' too
    const string tokens = "token";
          string body   = "{\"message\":\"Ciao! 你好！\"}";
    const string expected = "{\"header\":{\"success\":true},\"message\":\"Ciao! 你好！\"}";

    Sf1Driver driver(HOST, CONF);
    BOOST_CHECK_EQUAL(1, driver.getSequence());
    
    string response = driver.call(uri, tokens, body);
    BOOST_CHECK_EQUAL(2, driver.getSequence());
    BOOST_CHECK_EQUAL(expected, response);
    BOOST_CHECK(driver.getSequence() != 0);
}


bool
match(const string& s) {
    size_t pos = s.find("\"success\":");
    return (pos != string::npos) 
        && ("true" == s.substr(pos + 10, 4));
}

BOOST_AUTO_TEST_CASE(documents_search) {
    const string uri    = "/documents/search"; 
    const string tokens = "";
          string body   = "{ \"collection\":\"example\","
                          "  \"header\":{\"check_time\":true},"
                          "  \"search\":{\"keywords\":\"america\"},"
                          "  \"limit\":10"
                          "}";
    
    Sf1Driver driver(HOST, CONF);
    string response = driver.call(uri, tokens, body);
    BOOST_CHECK_EQUAL(2, driver.getSequence());
    BOOST_CHECK_PREDICATE(match, (response));    
}

#endif
