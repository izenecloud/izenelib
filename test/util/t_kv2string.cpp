#include <stdlib.h>
#include <string.h>
#include <util/kv2string.h>
#include <boost/test/unit_test.hpp>
#include <iostream>

using namespace izenelib::util;
using namespace std;

BOOST_AUTO_TEST_CASE(kv2string_test)
{
    std::cout << "begin test kv2string" << std::endl;
    std::string key="123##";
    std::string value = "$$456$$";

    std::string key2="##123##";
    std::string value2 = "$$456$$";

    std::string key3="%%##123##%%";
    std::string value3 = "$$%%456%%$$";

    kv2string test1;
    test1.setValue(key, value);
    test1.setValue(key2, value2);
    test1.setValue(key3, value3);

    std::string data = test1.serialize();
    test1.loadKvString(data);
    BOOST_CHECK(test1.getStrValue(key) == value);
    BOOST_CHECK(test1.getStrValue(key2) == value2);
    BOOST_CHECK(test1.getStrValue(key3) == value3);
}
