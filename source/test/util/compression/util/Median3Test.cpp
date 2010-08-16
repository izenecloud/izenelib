#include <boost/test/unit_test.hpp>

#include <util/compression/Median3.h>

#include <functional>

using namespace boost::unit_test;
using namespace izenelib::util::compression;

BOOST_AUTO_TEST_SUITE(util_Median3_test)

BOOST_AUTO_TEST_CASE(EqualElements_test)
{
    BOOST_CHECK(1 == median3(1, 1, 1));

    char items[] = {'a', 'a', 'a'};
    BOOST_CHECK('a' == median3(items[0], items[1], items[2]));
}

BOOST_AUTO_TEST_CASE(TwoEqualElements_test)
{
    BOOST_CHECK(1 == median3(1, 1, 2));

    char items[] = {'a', 'b', 'a'};
    BOOST_CHECK('a' == median3(items[0], items[1], items[2]));
}

BOOST_AUTO_TEST_CASE(Normal_test)
{
    int items[] = { 1, 2, 3 };

    BOOST_CHECK(2 == median3(items[0], items[1], items[2]));
}

BOOST_AUTO_TEST_CASE(EqualElementsCmp_test)
{
    BOOST_CHECK(1 == median3(1, 1, 1, std::greater<int>()));

    char items[] = {'a', 'a', 'a'};
    BOOST_CHECK('a' == median3(items[0], items[1], items[2], std::greater<char>()));
}

BOOST_AUTO_TEST_CASE(TwoEqualElementsCmp_test)
{
    BOOST_CHECK(1 == median3(1, 1, 2, std::greater<int>()));

    char items[] = {'a', 'b', 'a'};
    BOOST_CHECK('a' == median3(items[0], items[1], items[2], std::greater<char>()));
}

BOOST_AUTO_TEST_CASE(Cmp_test)
{
    int items[] = { 1, 2, 3 };

    BOOST_CHECK(2 == median3(items[0], items[1], items[2], std::greater<char>()));
}

BOOST_AUTO_TEST_SUITE_END() // util_Median3_test
