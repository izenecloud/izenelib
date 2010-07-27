#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>

#include <cstdio>
#include <map>
#include <set>
#include <string>
#include <algorithm>

#include <am/tc/raw/Hash.h>

#define DIR_PREFIX "./tmp/am_tc_raw_Iter_"

using namespace boost::unit_test;
using namespace izenelib::am::tc::raw;

typedef boost::mpl::list<Hash> test_types;

namespace { // {anonymous}
struct FillFixture
{
    FillFixture()
    {
        data["key1"] = "value1";
        data["key2"] = "value2";
        data["key3"] = "value3";
        keys.insert("key1");
        keys.insert("key2");
        keys.insert("key3");
    }

    static std::string bufferToString(const Buffer& buf)
    {
        return std::string(buf.data(), buf.size());
    }

    template<typename T>
    void fill1(T& t)
    {
        Buffer key;
        Buffer value;
        typedef std::map<std::string, std::string>::iterator iterator;

        iterator it = data.begin();
        key.attach(const_cast<char*>(it->first.c_str()), it->first.size());
        value.attach(const_cast<char*>(it->second.c_str()), it->second.size());
        t.insert(key, value);
    }

    template<typename T>
    void fill3(T& t)
    {
        Buffer key;
        Buffer value;
        typedef std::map<std::string, std::string>::iterator iterator;

        for (iterator it = data.begin(); it != data.end(); ++it)
        {
            key.attach(const_cast<char*>(it->first.c_str()), it->first.size());
            value.attach(const_cast<char*>(it->second.c_str()), it->second.size());
            t.insert(key, value);
        }
    }

    std::map<std::string, std::string> data;
    std::set<std::string> keys;
};
} // namespace {anonymous}


BOOST_FIXTURE_TEST_SUITE(tc_raw_Iter_test, FillFixture)

BOOST_AUTO_TEST_CASE_TEMPLATE(Empty_IterNextKey_test, T, test_types)
{
    T h(DIR_PREFIX "Empty_IterNextKey_test");
    BOOST_CHECK(h.open());

    BOOST_CHECK(h.iterInit());
    Buffer key;
    BOOST_CHECK(!h.iterNext(key));
    BOOST_CHECK(key.empty());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Empty_IterNext_test, T, test_types)
{
    std::remove(DIR_PREFIX "Empty_IterNext_test");
    T h(DIR_PREFIX "Empty_IterNext_test");
    BOOST_CHECK(h.open());

    BOOST_CHECK(h.iterInit());
    Buffer key;
    Buffer value;
    BOOST_CHECK(!h.iterNext(key, value));
    BOOST_CHECK(key.empty());
    BOOST_CHECK(value.empty());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Empty_GetFirstKey_test, T, test_types)
{
    std::remove(DIR_PREFIX "Empty_GetFirstKey_test");
    T h(DIR_PREFIX "Empty_GetFirstKey_test");
    BOOST_CHECK(h.open());

    Buffer key;
    BOOST_CHECK(!h.getFirst(key));
    BOOST_CHECK(key.empty());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Empty_GetFirst_test, T, test_types)
{
    std::remove(DIR_PREFIX "Empty_GetFirst_test");
    T h(DIR_PREFIX "Empty_GetFirst_test");
    BOOST_CHECK(h.open());

    Buffer key;
    Buffer value;
    BOOST_CHECK(!h.getFirst(key, value));
    BOOST_CHECK(key.empty());
    BOOST_CHECK(value.empty());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(One_IterNextKey_test, T, test_types)
{
    std::remove(DIR_PREFIX "One_IterNextKey_test");
    T h(DIR_PREFIX "One_IterNextKey_test");
    BOOST_CHECK(h.open());

    fill1(h);

    BOOST_REQUIRE(h.iterInit());

    Buffer key;
    BOOST_CHECK(h.iterNext(key));
    BOOST_CHECK(bufferToString(key) == *(keys.begin()));

    BOOST_CHECK(!h.iterNext(key));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(One_IterNext_test, T, test_types)
{
    std::remove(DIR_PREFIX "One_IterNext_test");
    T h(DIR_PREFIX "One_IterNext_test");
    BOOST_CHECK(h.open());

    fill1(h);

    BOOST_REQUIRE(h.iterInit());

    Buffer key;
    Buffer value;
    BOOST_CHECK(h.iterNext(key, value));
    BOOST_CHECK(bufferToString(key) == *(keys.begin()));
    BOOST_CHECK(bufferToString(value) == data.begin()->second);

    BOOST_CHECK(!h.iterNext(key, value));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(One_GetNextKey_test, T, test_types)
{
    std::remove(DIR_PREFIX "One_GetNextKey_test");
    T h(DIR_PREFIX "One_GetNextKey_test");
    BOOST_CHECK(h.open());

    fill1(h);

    Buffer key;
    BOOST_CHECK(h.getFirst(key));
    BOOST_CHECK(bufferToString(key) == *(keys.begin()));

    BOOST_CHECK(!h.getNext(key));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(One_GetNext_test, T, test_types)
{
    std::remove(DIR_PREFIX "One_GetNext_test");
    T h(DIR_PREFIX "One_GetNext_test");
    BOOST_CHECK(h.open());

    fill1(h);

    Buffer key;
    Buffer value;
    BOOST_CHECK(h.getFirst(key, value));
    BOOST_CHECK(bufferToString(key) == *(keys.begin()));
    BOOST_CHECK(bufferToString(value) == data.begin()->second);

    BOOST_CHECK(!h.getNext(key, value));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Three_IterNextKey_test, T, test_types)
{
    std::remove(DIR_PREFIX "Three_IterNextKey_test");
    T h(DIR_PREFIX "Three_IterNextKey_test");
    BOOST_CHECK(h.open());

    fill3(h);

    BOOST_REQUIRE(h.iterInit());

    std::set<std::string> fetchedKeys;

    Buffer key;
    while (h.iterNext(key))
    {
        fetchedKeys.insert(bufferToString(key));
    }

    BOOST_CHECK(fetchedKeys == keys);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Three_IterNext_test, T, test_types)
{
    std::remove(DIR_PREFIX "Three_IterNext_test");
    T h(DIR_PREFIX "Three_IterNext_test");
    BOOST_CHECK(h.open());

    fill3(h);

    BOOST_REQUIRE(h.iterInit());

    std::map<std::string, std::string> fetchedData;

    Buffer key;
    Buffer value;
    while (h.iterNext(key, value))
    {
        fetchedData[bufferToString(key)] = bufferToString(value);
    }

    BOOST_CHECK(fetchedData == data);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Three_GetNextKey_test, T, test_types)
{
    std::remove(DIR_PREFIX "Three_GetNextKey_test");
    T h(DIR_PREFIX "Three_GetNextKey_test");
    BOOST_CHECK(h.open());

    fill3(h);

    std::set<std::string> fetchedKeys;

    Buffer key;
    if (h.getFirst(key))
    {
        do
        {
            fetchedKeys.insert(bufferToString(key));
        } while (h.getNext(key));
    }

    BOOST_CHECK(fetchedKeys == keys);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Three_GetNext_test, T, test_types)
{
    std::remove(DIR_PREFIX "Three_GetNext_test");
    T h(DIR_PREFIX "Three_GetNext_test");
    BOOST_CHECK(h.open());

    fill3(h);

    std::map<std::string, std::string> fetchedData;

    Buffer key;
    Buffer value;
    if (h.getFirst(key, value))
    {
        do
        {
            fetchedData[bufferToString(key)] = bufferToString(value);
        } while (h.getNext(key, value));
    }

    BOOST_CHECK(fetchedData == data);
}

BOOST_AUTO_TEST_SUITE_END() // tc_raw_Iter_test
