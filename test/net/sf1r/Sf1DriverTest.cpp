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
#include <boost/foreach.hpp>

using namespace NS_IZENELIB_SF1R;
using namespace std;


BOOST_AUTO_TEST_CASE(connection_fail) {
    const Sf1Config conf;
    
    vector<string> hosts;
    hosts.push_back("somewhere");
    hosts.push_back("somewhere:18181");
    hosts.push_back("somewhere:service");
    
    BOOST_FOREACH(const string& host, hosts) {
        Sf1Driver* driver = NULL;
        BOOST_CHECK_NO_THROW(driver = new Sf1Driver(host, conf));
        
        string request = "{\"message\":\"Ciao! 你好！\"}";
        BOOST_CHECK_THROW(driver->call("test/echo", "", request), NetworkError);
        
        delete driver;
    }
}


/*
 * This test requires a running SF1 instance.
 */

#ifdef ENABLE_SF1_TEST


string HOST = "localhost:18181";
Sf1Config CONF;


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


BOOST_AUTO_TEST_CASE(reconnect) {
    const string uri    = "test/echo";
    const string tokens = "token";
          string body   = "{\"message\":\"Ciao! 你好！\"}";
    
    CONF.initialSize = 2;
    Sf1Driver driver(HOST, CONF);
    
    // use twice all the connection in the pool
    for (size_t i = 0; i < 2 * CONF.initialSize; i++) {
        BOOST_CHECK_NO_THROW(driver.call(uri, tokens, body));
    }
    
    cout << endl << "Stop the SF1 and then press ENTER ... ";
    cout.flush(); 
    cin.get();
    cout << "continue!" << endl << endl;
    
    // ensure that the invalid connections are discarded and cannot estabilish 
    // more connections while the host is down or the connection is invalid
    for (size_t i = 0; i < 2 * CONF.initialSize; i++) {
        BOOST_CHECK_THROW(driver.call(uri, tokens, body), NetworkError);
    }
    
    cout << endl << "Press ENTER when the SF1 has completely restarted ... ";
    cout.flush(); 
    cin.get();
    cout << "continue!" << endl << endl;
    
    // use again twice all the connection in the pool
    for (size_t i = 0; i < 2 * CONF.initialSize; i++) {
        BOOST_CHECK_NO_THROW(driver.call(uri, tokens, body));
    }
}


BOOST_AUTO_TEST_CASE(reconnect2) {
    const string uri    = "test/echo";
    const string tokens = "token";
          string body   = "{\"message\":\"Ciao! 你好！\"}";
    
    CONF.initialSize = 2;
    Sf1Driver driver(HOST, CONF);
    
    // use twice all the connection in the pool
    for (size_t i = 0; i < 2 * CONF.initialSize; i++) {
        BOOST_CHECK_NO_THROW(driver.call(uri, tokens, body));
    }
    
    cout << endl << "Restart the SF1 and then press ENTER ... ";
    cout.flush(); 
    cin.get();
    cout << "continue!" << endl << endl;
    
    // use again twice all the connection in the pool
    for (size_t i = 0; i < 2 * CONF.initialSize; i++) {
        BOOST_CHECK_NO_THROW(driver.call(uri, tokens, body));
    }
}

#endif
