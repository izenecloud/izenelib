#include <boost/test/unit_test.hpp>

#include <util/compression/PseudoMedian.h>

using namespace boost::unit_test;
using namespace izenelib::util::compression;

BOOST_AUTO_TEST_SUITE(util_PseudoMedian_test)

BOOST_AUTO_TEST_CASE(SmallArray_test)
{
    int a[] = { 2, 2, 2, 2, 2, 2, 2 };
    unsigned n = sizeof(a) / sizeof(int);

    BOOST_ASSERT(2 == pseudoMedian(a, a + n));

    a[3] = 10;
    BOOST_ASSERT(10 == pseudoMedian(a, a + n));
}

BOOST_AUTO_TEST_CASE(MiddleArray_test)
{
    // 0, 4, 8
    int a[] = { 2, 2, 2, 2, 2, 2, 2, 2, 2 };
    unsigned n = sizeof(a) / sizeof(int);

    BOOST_ASSERT(2 == pseudoMedian(a, a + n));

    a[0] = 3;
    a[4] = 30;
    a[8] = 300;
    BOOST_ASSERT(30 == pseudoMedian(a, a + n));

    a[0] = 40;
    BOOST_ASSERT(40 == pseudoMedian(a, a + n));

    a[8] = 35;
    BOOST_ASSERT(35 == pseudoMedian(a, a + n));
}

BOOST_AUTO_TEST_CASE(LargeArray_test)
{
    int a[] = {
        0,  1,  2,  3,  4,  5,
        6,  7,  8,  9,  10, 11,
        12, 13, 14, 15, 16, 17,
        18, 19, 20, 21, 22, 23,
        24, 25, 26, 27, 28, 29,
        30, 31, 32, 33, 34, 35,
        36, 37, 38, 39, 40, 41,
        42, 43, 44, 45, 46, 47,
        48
    };
    unsigned n = sizeof(a) / sizeof(int);

    // 0,  6,  12 => 6  -+
    // 18, 24, 30 => 24  +-> 24
    // 36, 42, 48 => 42 -+
    BOOST_ASSERT(24 == pseudoMedian(a, a + n));

    // 0,  6,   12 => 6  -+
    // 18, 300, 30 => 30  +-> 30
    // 36, 42,  48 => 42 -+
    a[24] = 300;
    BOOST_ASSERT(30 == pseudoMedian(a, a + n));

    // 32, 6,   120 => 32 -+
    // 18, 300, 30  => 30  +-> 32
    // 36, 42,  48  => 42 -+
    a[0] = 32;
    a[12] = 120;
    BOOST_ASSERT(32 == pseudoMedian(a, a + n));
}

BOOST_AUTO_TEST_SUITE_END() // util_PseudoMedian_test
