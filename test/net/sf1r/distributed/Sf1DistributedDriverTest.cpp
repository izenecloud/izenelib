/* 
 * File:   Sf1DistributedDriverTest.cpp
 * Author: Paolo D'Apice
 * 
 * Created on March 1, 2012, 4:03 PM
 */

#define BOOST_TEST_MODULE Sf1DistributedDriverTest
#include <boost/test/unit_test.hpp>

#include "../common.h"
#include "net/sf1r/distributed/Sf1DistributedDriver.hpp"
#include <string>

using namespace NS_IZENELIB_SF1R;
using namespace std;


BOOST_AUTO_TEST_CASE(connection_fail) {
    const string host = "somewhere";
    const Sf1Config conf;
    
    try {
        Sf1DistributedDriver driver(host, conf);
        BOOST_FAIL("ServerError expected");
    } catch(ServerError& e) {
    }
}


#ifdef ENABLE_SF1_TEST // don't know if there is a running SF1


const string HOSTS = "localhost:2181";
const Sf1Config CONF;

BOOST_AUTO_TEST_CASE(echo) {
    const string uri    = "test/echo";
    const string tokens = "token";
          string body   = "{\"message\":\"Ciao! 你好！\"}";
    const string expected = "{\"header\":{\"success\":true},\"message\":\"Ciao! 你好！\"}";

    Sf1DistributedDriver driver(HOSTS, CONF);
    BOOST_CHECK_EQUAL(1, driver.getSequence());
    
    string response = driver.call(uri, tokens, body);
    BOOST_CHECK_EQUAL(2, driver.getSequence());
    BOOST_CHECK_EQUAL(expected, response);
    BOOST_CHECK(driver.getSequence() != 0);
}


BOOST_AUTO_TEST_CASE(echo_collection) {
    const string uri    = "test/echo";
    const string tokens = "token";
          string body   = "{\"collection\":\"b5mm\",\"message\":\"Ciao! 你好！\"}";
    const string expected = "{\"header\":{\"success\":true},\"message\":\"Ciao! 你好！\"}";

    Sf1DistributedDriver driver(HOSTS, CONF);
    BOOST_CHECK_EQUAL(1, driver.getSequence());
    
    string response = driver.call(uri, tokens, body);
    BOOST_CHECK_EQUAL(2, driver.getSequence());
    BOOST_CHECK_EQUAL(expected, response);
    BOOST_CHECK(driver.getSequence() != 0);
}


#if 0
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
    
    Sf1DistributedDriver driver(HOSTS, CONF);
    string response = driver.call(uri, tokens, body);
    BOOST_CHECK_EQUAL(2, driver.getSequence());
    BOOST_CHECK_PREDICATE(match, (response));    
}
#endif
#endif
