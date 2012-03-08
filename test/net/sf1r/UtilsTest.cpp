/* 
 * File:   UtilsTest.cpp
 * Author: Paolo D'Apice
 * 
 * Created on February 20, 2012, 10:53 AM
 */

#define BOOST_TEST_MODULE UtilsTest
#include <boost/test/unit_test.hpp>

#include "net/sf1r/Utils.hpp"
#include <string>
#include <vector>

using namespace NS_IZENELIB_SF1R;
using namespace std;

BOOST_AUTO_TEST_SUITE(test_split)

BOOST_AUTO_TEST_CASE(split_string) {
    const string input   = "this,is,a, ,string,,to,be splitted";
    vector<string> output;
    vector<string> expected;
    expected.push_back("this");
    expected.push_back("is");
    expected.push_back("a");
    expected.push_back(" ");
    expected.push_back("string");
    expected.push_back("to");
    expected.push_back("be splitted");
        
    // split using a correct delimiter
    split(input, ',', output);
    BOOST_CHECK_EQUAL(expected.size(), output.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        BOOST_CHECK_EQUAL(expected[i], output[i]);
    }
    
    // split using a wrong delimiter
    output.clear();
    split(input, ':', output);
    BOOST_CHECK_EQUAL(1, output.size());
    BOOST_CHECK_EQUAL(input, output.front());
}

BOOST_AUTO_TEST_SUITE_END()
