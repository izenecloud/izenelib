/* 
 * File:   RapidjsonTest.cpp
 * Author: Paolo D'Apice
 * 
 * Created on January 9, 2012, 4:24 PM
 */

#define BOOST_TEST_MODULE RapidjsonTest
#include <boost/test/unit_test.hpp>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

using namespace rapidjson;
using namespace std;
        

BOOST_AUTO_TEST_CASE(rapidjson_test) {
    // sample data
    string body = "{\"message\":\"Ciao! 你好！\"}";
    string ctrl = "test";
    const int len = 5;
    char actn[len] = "echo";
    //cout << "Original: " << body << endl;
    
    // parse
    Document document;
#if 0 // manually use buffer
    char buffer[body.length()];
    memcpy(buffer, body.c_str(), body.length());
    
    if (document.ParseInsitu<0>(buffer).HasParseError()) {
        BOOST_FAIL("Parse error");
    }
#else // automatically use buffer
    if (document.Parse<0>(body.c_str()).HasParseError()) {
        BOOST_FAIL("Parse error");
    }
#endif
    
    Document::AllocatorType& allocator = document.GetAllocator();
    
    // access values
    BOOST_CHECK(document["message"].IsString());
    BOOST_CHECK(document["header"].IsNull());
    
    Value& v = document["no"];
    BOOST_CHECK(v.IsNull());
    v = document["message"];
    BOOST_CHECK(not v.IsNull());
    
    // create values
    Value controller(ctrl.c_str(), ctrl.length(), allocator);
    Value action(actn, len - 1, allocator);
    
    Value header;
    header.SetObject();
    header.AddMember("controller", controller, allocator);
    header.AddMember("action", action, allocator);
    document.AddMember("header", header, allocator);
    
    BOOST_CHECK(header.IsNull());
    BOOST_CHECK(document["header"].IsObject());
    
    // add values
    Value& headerRef = document["header"];
    Value check("yes", sizeof("yes") - 1, allocator);
    headerRef.AddMember("check", check, allocator);
    
    // output
    StringBuffer stream;
    Writer<StringBuffer> writer(stream);
    document.Accept(writer);
    //cout << "Edited: " << string(stream.GetString(), stream.GetSize()) << endl;
}


BOOST_AUTO_TEST_CASE(valid_test) {
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
        Document document;
        document.Parse<0>(it->c_str());
        BOOST_CHECK(not document.HasParseError());
    }
}


BOOST_AUTO_TEST_CASE(invalid_test) {
    vector<string> types;
    types.push_back("");
    types.push_back(" ");
    types.push_back("{");
    types.push_back(" } ");
    types.push_back("{\"hello\"}");
    types.push_back("{\"hello\":}");
    types.push_back("{:\"hello\"}");
    types.push_back("{hello:world}");
    
    for (vector<string>::iterator it = types.begin(); it < types.end(); it++) {
        Document document;
        document.Parse<0>(it->c_str());
        BOOST_CHECK(document.HasParseError());
    }
}
