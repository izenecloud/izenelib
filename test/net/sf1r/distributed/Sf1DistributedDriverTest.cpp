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

/*
 * This test requires a running ZooKeeper server and some SF1 instances.
 * 
 * This is an integration test proving the correctness of the integration
 * of all the building blocks used to assembly the distributed driver.
 * Please refer to the other tests in this folder.
 */

#if defined(ENABLE_SF1_TEST) && defined(ENABLE_ZK_TEST)


const string HOSTS = "localhost:2181";
const Sf1Config CONF;
const int LOOP = 5;


/* Test fixture. */
struct Tester {
    Tester() : driver(HOSTS, CONF) {}
    
    void
    doTest(const string& uri, const string& tokens, 
                string& body, const string& expected) {
        string response = driver.call(uri, tokens, body);
        BOOST_CHECK_EQUAL(expected, response);
        BOOST_CHECK(driver.getSequence() > 0);
    }
    
private:
    Sf1DistributedDriver driver;
};


/** Performs a series of test/echo requests. */
BOOST_FIXTURE_TEST_CASE(echo, Tester) {
    const string uri    = "test/echo";
    const string tokens = "token";
          string body   = "{\"message\":\"Ciao! 你好！\"}";
    const string expected = "{\"header\":{\"success\":true},\"message\":\"Ciao! 你好！\"}";

    for (int i = 0; i < LOOP; i++) {
        doTest(uri, tokens, body, expected);
    }
}


/** Performs a series of test/echo requests specifying a collection. */
BOOST_FIXTURE_TEST_CASE(echo_collection, Tester) {
    const string uri    = "test/echo";
    const string tokens = "token";
          string body   = "{\"collection\":\"example\",\"message\":\"Ciao! 你好！\"}";
    const string expected = "{\"header\":{\"success\":true},\"message\":\"Ciao! 你好！\"}";

    for (int i = 0; i < LOOP; i++) {
        doTest(uri, tokens, body, expected);
    }
}


/** No route for the given collection. */
BOOST_FIXTURE_TEST_CASE(echo_collection_none, Tester) {
    const string uri    = "test/echo";
    const string tokens = "token";
          string body   = "{\"collection\":\"some\",\"message\":\"Ciao! 你好！\"}";
    const string expected = "{\"header\":{\"success\":true},\"message\":\"Ciao! 你好！\"}";

    BOOST_CHECK_THROW(doTest(uri, tokens, body, expected), RoutingError);
}


bool
match(const string& s) {
    size_t pos = s.find("\"success\":");
    return (pos != string::npos) 
        && ("true" == s.substr(pos + 10, 4));
}

void
sendRequests(Sf1DistributedDriver& driver, const string& uri,
        const string& tokens, string& body) {
    for (int i = 0; i < LOOP; i++) {
        string response = driver.call(uri, tokens, body);
        BOOST_CHECK(driver.getSequence() > 0);
        BOOST_CHECK_PREDICATE(match, (response));
    }
}


/**
 * In order to run this test you need a ZooKeeper server and at least
 * two SF1 running.
 * 
 * Unfortunately there is not (yet?) automatic procedure, so here is:
 * - run the first SF1 and start the test
 * - when the test pauses, start the second SF1
 * - when the test pauses again, stop one SF1
 * Inspecting the logs you can verify that the routing takes place.
 */
BOOST_AUTO_TEST_CASE(documents_search) {
    const string uri    = "/documents/search"; 
    const string tokens = "";
          string body   = "{ \"collection\":\"b5mm\","
                          "  \"header\":{\"check_time\":true},"
                          "  \"search\":{\"keywords\":\"america\"},"
                          "  \"limit\":10"
                          "}";
    
    Sf1DistributedDriver driver(HOSTS, CONF);
    sendRequests(driver, uri, tokens, body);
    
    cout << "\n\n*** Now you should turn ON one SF1 instance! ***\n\n" << endl;
    sleep(20);
    
    sendRequests(driver, uri, tokens, body);
    
    cout << "\n\n*** Now you should turn OFF one SF1 instance! ***\n\n" << endl;
    sleep(20);
    
    sendRequests(driver, uri, tokens, body);
}


/** No route for the given collection. */
BOOST_AUTO_TEST_CASE(documents_search_fail) {
    const string uri    = "/documents/search"; 
    const string tokens = "";
          string body   = "{ \"collection\":\"some\","
                          "  \"header\":{\"check_time\":true},"
                          "  \"search\":{\"keywords\":\"america\"},"
                          "  \"limit\":10"
                          "}";
    
    Sf1DistributedDriver driver(HOSTS, CONF);
    BOOST_CHECK_THROW(driver.call(uri, tokens, body), RoutingError);
}


#endif
