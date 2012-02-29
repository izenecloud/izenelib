/* 
 * File:   JsonWriterTest.cpp
 * Author: Paolo D'Apice
 * 
 * Created on January 10, 2012, 11:25 AM
 */

#define BOOST_TEST_MODULE JsonWriterTest
#include <boost/test/unit_test.hpp>

#include "net/sf1r/JsonWriter.hpp"
#include <iostream>

using namespace NS_IZENELIB_SF1R;
using namespace std;

typedef const char* cc;
struct TestRequest {
    string controller;
    string action;
    string tokens;
    string request;
    string collection;
    string expected;
    TestRequest(cc c, cc a, cc t, cc r, cc x, cc e)
            : controller(c), action(a), tokens(t), request(r), collection(x), expected(e) {};
};

BOOST_AUTO_TEST_CASE(setHeader_test) {
    vector<TestRequest> tests;
    // all data present
    tests.push_back(TestRequest("test","echo","token",
            "{\"message\":\"Ciao! 你好！\",\"collection\":\"test\"}", "test",
            "{\"message\":\"Ciao! 你好！\",\"collection\":\"test\",\"header\":{\"controller\":\"test\",\"action\":\"echo\",\"acl_tokens\":\"token\"}}"));
    // no tokens
    tests.push_back(TestRequest("test","echo","",
            "{\"message\":\"Ciao! 你好！\",\"collection\":\"test\"}", "test",
            "{\"message\":\"Ciao! 你好！\",\"collection\":\"test\",\"header\":{\"controller\":\"test\",\"action\":\"echo\"}}"));
    // no action
    tests.push_back(TestRequest("test","","token",
            "{\"message\":\"Ciao! 你好！\",\"collection\":\"tost\"}", "tost",
            "{\"message\":\"Ciao! 你好！\",\"collection\":\"tost\",\"header\":{\"controller\":\"test\",\"acl_tokens\":\"token\"}}"));
    // no action no tokens
    tests.push_back(TestRequest("test","","",
            "{\"message\":\"Ciao! 你好！\",\"collection\":\"tost\"}", "tost",
            "{\"message\":\"Ciao! 你好！\",\"collection\":\"tost\",\"header\":{\"controller\":\"test\"}}"));
    // header already present
    tests.push_back(TestRequest("test","","",
            "{\"collection\":\"\",\"message\":\"Ciao! 你好！\",\"header\":{\"check_time\":true}}", "",
            "{\"collection\":\"\",\"message\":\"Ciao! 你好！\",\"header\":{\"check_time\":true,\"controller\":\"test\"}}"));
    // header already present
    tests.push_back(TestRequest("test","","token",
            "{\"message\":\"Ciao! 你好！\",\"header\":{\"check_time\":true}}", "",
            "{\"message\":\"Ciao! 你好！\",\"header\":{\"check_time\":true,\"controller\":\"test\",\"acl_tokens\":\"token\"}}"));
    
    
    JsonWriter writer;
    for (vector<TestRequest>::iterator it = tests.begin(); it < tests.end(); it++) {
        string coll;
        writer.setHeader(it->controller, it->action, it->tokens, it->request, coll);
        BOOST_CHECK_EQUAL(it->expected, it->request);
        BOOST_CHECK_EQUAL(it->collection, coll);
    }
}


// basically repeats the tests in RapijsonTest.cpp test case
BOOST_AUTO_TEST_CASE(checkData_invalid_test) {
    JsonWriter writer;
    
    // bad formats 
    vector<string> types;
    types.push_back("");
    types.push_back(" ");
    types.push_back("{");
    types.push_back(" } ");
    types.push_back("{\"hello\"}");
    types.push_back("{\"hello\":}");
    types.push_back("{:\"hello\"}");
    types.push_back("{hello:world}");
    types.push_back("trail{\"hello\":\"world\"}");
    types.push_back("{\"hello\":\"world\"}trail");
    
    for (vector<string>::iterator it = types.begin(); it < types.end(); it++) {
        BOOST_CHECK(not writer.checkData(*it));
    }
}

// basically repeats the tests in RapijsonTest.cpp test case
BOOST_AUTO_TEST_CASE(checkData_valid_test) {
    JsonWriter writer;
    
    // good formats
    vector<string> types;
    types.push_back("{}");
    types.push_back("{\"hello\":\"world\"}");
    types.push_back("{\"t\":true,\"f\":false}");
    types.push_back("{\"n\":null}");
    types.push_back("{\"i\":123}");
    types.push_back("{\"pi\":3.1416}");
    types.push_back("{\"a\":[1,2,3,4]}");
    types.push_back("{\"hello\":\"world\",\"t\":true,\"f\":false,\"n\":null,\"i\":123,\"pi\":3.1416,\"a\":[1,2,3,4]}");
    
    for (vector<string>::iterator it = types.begin(); it < types.end(); it++) {
        BOOST_CHECK(writer.checkData(*it));
    }
}
