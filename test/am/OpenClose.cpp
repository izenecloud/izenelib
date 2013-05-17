#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>
#include <am/am_test/am_types.h>

#include <cstdio>

#define DIR_PREFIX "./tmp/am_OpenClose_"

using namespace izenelib::am;

typedef boost::mpl::list<
    tc::Hash<int, std::string>,
    tc::BTree<int, std::string>
    , sdb_hash<int, std::string>
> test_types;

BOOST_AUTO_TEST_SUITE(OpenClose_test)

BOOST_AUTO_TEST_CASE_TEMPLATE(DefaultConstructor_test, T, test_types)
{
    T h;
    BOOST_CHECK(!h.is_open());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Open_test, T, test_types)
{
    std::remove(DIR_PREFIX "Open_test");
    T h(DIR_PREFIX "Open_test");
    bool opened;
    BOOST_WARN(opened = h.open());

    if (opened)
    {
        BOOST_CHECK(h.is_open());
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Close_test, T, test_types)
{
    std::remove(DIR_PREFIX "Close_test");
    T h(DIR_PREFIX "Close_test");
    BOOST_WARN(h.open());

    h.close();
    BOOST_CHECK(!h.is_open());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(CloseWithoutOpen_test, T, test_types)
{
    std::remove(DIR_PREFIX "CloseWithoutOpen_test");
    T h(DIR_PREFIX "CloseWithoutOpen_test");
    h.close();
    BOOST_CHECK(!h.is_open());
}

BOOST_AUTO_TEST_SUITE_END()
