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
    
    BOOST_CHECK_THROW(new Sf1DistributedDriver(host, conf), ServerError);
}


#ifdef ENABLE_SF1_TEST // don't know if there is a running SF1


const string HOSTS = "localhost:2181";
const Sf1Config CONF;
const int LOOP = 5;

struct Tester {
    Tester() : driver(HOSTS, CONF) {}
    
    void
    doTest(const string& uri, const string& tokens, 
                string& body, const string& expected) {
        string response = driver.call(uri, tokens, body);
        BOOST_CHECK_EQUAL(expected, response);
        BOOST_CHECK(driver.getSequence() != 0);
    }
    
private:
    Sf1DistributedDriver driver;
};


BOOST_FIXTURE_TEST_CASE(echo, Tester) {
    const string uri    = "test/echo";
    const string tokens = "token";
          string body   = "{\"message\":\"Ciao! 你好！\"}";
    const string expected = "{\"header\":{\"success\":true},\"message\":\"Ciao! 你好！\"}";

    for (int i = 0; i < LOOP; i++) {
        doTest(uri, tokens, body, expected);
    }
}


BOOST_FIXTURE_TEST_CASE(echo_collection, Tester) {
    const string uri    = "test/echo";
    const string tokens = "token";
          string body   = "{\"collection\":\"b5mm\",\"message\":\"Ciao! 你好！\"}";
    const string expected = "{\"header\":{\"success\":true},\"message\":\"Ciao! 你好！\"}";

    for (int i = 0; i < LOOP; i++) {
        doTest(uri, tokens, body, expected);
    }
}


BOOST_FIXTURE_TEST_CASE(echo_collection_none, Tester) {
    const string uri    = "test/echo";
    const string tokens = "token";
          string body   = "{\"collection\":\"some\",\"message\":\"Ciao! 你好！\"}";
    const string expected = "{\"header\":{\"success\":true},\"message\":\"Ciao! 你好！\"}";

    BOOST_CHECK_THROW(doTest(uri, tokens, body, expected), RoutingError);
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
