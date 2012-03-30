/* 
 * File:   RegexLexerTest.cpp
 * Author: Paolo D'Apice
 * 
 * Created on March 27, 2012, 3:10 PM
 */

#define BOOST_TEST_MODULE ConnectionPoolTest
#include <boost/test/unit_test.hpp>

#include "net/sf1r/distributed/RegexLexer.hpp"
#include <boost/foreach.hpp>
#include <string>
#include <vector>

using namespace NS_IZENELIB_SF1R;
using namespace std;


BOOST_AUTO_TEST_CASE(sanity) {
    const string URI_PATTERN = "^\\/\\w+(\\/\\w+)?$";
    RegexLexer lexer(URI_PATTERN);
    
    BOOST_CHECK_EQUAL(URI_PATTERN, lexer.regex());
    
    vector<string> good;
    good.push_back("/test");
    good.push_back("/test/echo");
    good.push_back("/recommend");
    good.push_back("/recommend/visit_item");
    
    BOOST_FOREACH(const string& input, good) {
        BOOST_CHECK(lexer.match(input));
    }
    
    vector<string> bad;
    bad.push_back("");
    bad.push_back(" ");
    bad.push_back("/");
    bad.push_back("/test/");
    bad.push_back("/test/echo/");
    bad.push_back("/recommend/visit_item/");
    
    BOOST_FOREACH(const string& input, bad) {
        BOOST_CHECK(not lexer.match(input));
    }
}


BOOST_AUTO_TEST_CASE(test_uri) {
    const string URI_PATTERN = "^test(\\/\\w+)?$";
    RegexLexer lexer(URI_PATTERN);
    
    BOOST_CHECK_EQUAL(URI_PATTERN, lexer.regex());
    
    vector<string> good;
    good.push_back("test");
    good.push_back("test/echo");
    
    BOOST_FOREACH(const string& input, good) {
        BOOST_CHECK(lexer.match(input));
    }
    
    vector<string> bad;
    bad.push_back("");
    bad.push_back(" ");
    bad.push_back("/");
    bad.push_back("/test/");
    bad.push_back("/test/echo");
    bad.push_back("recommend");
    bad.push_back("recommend/visit_item");
    bad.push_back("recommend/visit_item/");
    
    BOOST_FOREACH(const string& input, bad) {
        BOOST_CHECK(not lexer.match(input));
    }
}

