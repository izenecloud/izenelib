#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>

#include <am/tc/Hash.h>

#include <string>
#include <cstdio>

#define DIR_PREFIX "./tmp/am_tc_Iter_"

using namespace boost::unit_test;
using namespace izenelib::am::tc;

typedef boost::mpl::list<
    Hash<std::string, std::string>
> test_types;

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

    template<typename T>
    void fill1(T& t)
    {
        typedef std::map<std::string, std::string>::iterator iterator;

        iterator it = data.begin();
        t.insert(it->first, it->second);
    }

    template<typename T>
    void fill3(T& t)
    {
        typedef std::map<std::string, std::string>::iterator iterator;

        for (iterator it = data.begin(); it != data.end(); ++it)
        {
            t.insert(it->first, it->second);
        }
    }

    std::map<std::string, std::string> data;
    std::set<std::string> keys;
};
} // namespace {anonymous}


BOOST_FIXTURE_TEST_SUITE(tc_Iter_test, FillFixture)

BOOST_AUTO_TEST_CASE_TEMPLATE(Empty_IterNextKey_test, T, test_types)
{
    T h(DIR_PREFIX "Empty_IterNextKey_test");
    BOOST_CHECK(h.open());

    BOOST_CHECK(h.iterInit());
    std::string key;
    BOOST_CHECK(!h.iterNext(key));
    BOOST_CHECK(key.empty());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Empty_IterNext_test, T, test_types)
{
    std::remove(DIR_PREFIX "Empty_IterNext_test");
    T h(DIR_PREFIX "Empty_IterNext_test");
    BOOST_CHECK(h.open());

    BOOST_CHECK(h.iterInit());
    std::string key;
    std::string value;
    BOOST_CHECK(!h.iterNext(key, value));
    BOOST_CHECK(key.empty());
    BOOST_CHECK(value.empty());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Empty_GetFirstKey_test, T, test_types)
{
    std::remove(DIR_PREFIX "Empty_GetFirstKey_test");
    T h(DIR_PREFIX "Empty_GetFirstKey_test");
    BOOST_CHECK(h.open());

    std::string key;
    BOOST_CHECK(!h.getFirst(key));
    BOOST_CHECK(key.empty());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Empty_GetFirst_test, T, test_types)
{
    std::remove(DIR_PREFIX "Empty_GetFirst_test");
    T h(DIR_PREFIX "Empty_GetFirst_test");
    BOOST_CHECK(h.open());

    std::string key;
    std::string value;
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

    std::string key;
    BOOST_CHECK(h.iterNext(key));
    BOOST_CHECK(key == *(keys.begin()));

    BOOST_CHECK(!h.iterNext(key));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(One_IterNext_test, T, test_types)
{
    std::remove(DIR_PREFIX "One_IterNext_test");
    T h(DIR_PREFIX "One_IterNext_test");
    BOOST_CHECK(h.open());

    fill1(h);

    BOOST_REQUIRE(h.iterInit());

    std::string key;
    std::string value;
    BOOST_CHECK(h.iterNext(key, value));
    BOOST_CHECK(key == *(keys.begin()));
    BOOST_CHECK(value == data.begin()->second);

    BOOST_CHECK(!h.iterNext(key, value));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(One_GetNextKey_test, T, test_types)
{
    std::remove(DIR_PREFIX "One_GetNextKey_test");
    T h(DIR_PREFIX "One_GetNextKey_test");
    BOOST_CHECK(h.open());

    fill1(h);

    std::string key;
    BOOST_CHECK(h.getFirst(key));
    BOOST_CHECK(key == *(keys.begin()));

    BOOST_CHECK(!h.getNext(key));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(One_GetNext_test, T, test_types)
{
    std::remove(DIR_PREFIX "One_GetNext_test");
    T h(DIR_PREFIX "One_GetNext_test");
    BOOST_CHECK(h.open());

    fill1(h);

    std::string key;
    std::string value;
    BOOST_CHECK(h.getFirst(key, value));
    BOOST_CHECK(key == *(keys.begin()));
    BOOST_CHECK(value == data.begin()->second);

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

    std::string key;
    while (h.iterNext(key))
    {
        fetchedKeys.insert(key);
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

    std::string key;
    std::string value;
    while (h.iterNext(key, value))
    {
        fetchedData[key] = value;
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

    std::string key;
    if (h.getFirst(key))
    {
        do
        {
            fetchedKeys.insert(key);
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

    std::string key;
    std::string value;
    if (h.getFirst(key, value))
    {
        do
        {
            fetchedData[key] = value;
        } while (h.getNext(key, value));
    }

    BOOST_CHECK(fetchedData == data);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Three_Range_test, T, test_types)
{
    std::remove(DIR_PREFIX "Three_Range_test");
    T h(DIR_PREFIX "Three_Range_test");
    BOOST_CHECK(h.open());

    fill3(h);

    std::map<std::string, std::string> fetchedData;

    typename T::range_type range;
    for (h.all(range); !range.empty(); range.popFront())
    {
        fetchedData[range.frontKey()] = range.frontValue();
    }

    BOOST_CHECK(fetchedData == data);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Three_ExclusiveRange_test, T, test_types)
{
    std::remove(DIR_PREFIX "Three_ExclusiveRange_test");
    T h(DIR_PREFIX "Three_ExclusiveRange_test");
    BOOST_CHECK(h.open());

    fill3(h);

    std::map<std::string, std::string> fetchedData;

    typename T::exclusive_range_type range;
    for (h.exclusiveAll(range); !range.empty(); range.popFront())
    {
        fetchedData[range.frontKey()] = range.frontValue();
    }

    BOOST_CHECK(fetchedData == data);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(InitNotExisted_test, T, test_types)
{
    std::remove(DIR_PREFIX "InitNotExisted_test");
    T h(DIR_PREFIX "InitNotExisted_test");
    BOOST_CHECK(h.open());

    fill3(h);
    h.iterInit();

    BOOST_CHECK(!h.iterInit("notexisted"));
}

BOOST_AUTO_TEST_SUITE_END() // tc_Iter_test
