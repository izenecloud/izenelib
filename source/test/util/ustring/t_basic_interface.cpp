#include <boost/test/unit_test.hpp>
#include <util/ustring/UString.h>
#include <algorithm>

#define scoped_alloc NULL_ALLOCATOR

using namespace std;
using namespace izenelib::util;
using namespace boost::unit_test;
using namespace boost;

BOOST_AUTO_TEST_SUITE(BasicInterface_test)

BOOST_AUTO_TEST_CASE(DefaultConstruct_test)
{
    UString ustr;
    BOOST_CHECK_EQUAL(0U, ustr.length());
    BOOST_CHECK_EQUAL(0U, ustr.size());
    BOOST_CHECK_EQUAL(UString::UNKNOWN, ustr.getSystemEncodingType());
    BOOST_CHECK_EQUAL(0, ustr.getReferCnt());
}

BOOST_AUTO_TEST_CASE(EmptyCopyConstruct_test)
{
    UString orig;
    UString ustr(orig);

    BOOST_CHECK_EQUAL(0U, orig.length());
    BOOST_CHECK_EQUAL(0U, orig.size());
    BOOST_CHECK_EQUAL(UString::UNKNOWN, orig.getSystemEncodingType());
    BOOST_CHECK_EQUAL(0, orig.getReferCnt());

    BOOST_CHECK_EQUAL(0U, ustr.length());
    BOOST_CHECK_EQUAL(0U, ustr.size());
    BOOST_CHECK_EQUAL(UString::UNKNOWN, ustr.getSystemEncodingType());
    BOOST_CHECK_EQUAL(0, ustr.getReferCnt());

    BOOST_CHECK(orig == ustr);
}

BOOST_AUTO_TEST_CASE(StdStringConstruct_test)
{
    string str = "01234";
    UString ustr(str, UString::UTF_8);
    BOOST_CHECK_EQUAL(5U, ustr.length());
    BOOST_CHECK_EQUAL(5U * sizeof(uint16_t), ustr.size());
    BOOST_CHECK(std::equal(ustr.begin(), ustr.end(), str.begin()));
}

BOOST_AUTO_TEST_CASE(CopyConstruct_test)
{
    string str = "01234";
    UString orig(str, UString::UTF_8);
    UString ustr(orig);

    BOOST_CHECK_EQUAL(5U, ustr.length());
    BOOST_CHECK_EQUAL(5U * sizeof(uint16_t), ustr.size());
    BOOST_CHECK(std::equal(ustr.begin(), ustr.end(), str.begin()));

    BOOST_CHECK_EQUAL(5U, orig.length());
    BOOST_CHECK_EQUAL(5U * sizeof(uint16_t), orig.size());
    BOOST_CHECK(std::equal(orig.begin(), orig.end(), str.begin()));

    BOOST_CHECK(orig == ustr);
}

BOOST_AUTO_TEST_CASE(CStringConstruct_test)
{
    const char* str = "01234";
    UString ustr(str, UString::UTF_8);
    BOOST_CHECK_EQUAL(5U, ustr.length());
    BOOST_CHECK_EQUAL(5U * sizeof(uint16_t), ustr.size());
    BOOST_CHECK(std::equal(ustr.begin(), ustr.end(), str));
}

BOOST_AUTO_TEST_SUITE_END() // BasicInterface_TestSuite

#undef scoped_alloc
