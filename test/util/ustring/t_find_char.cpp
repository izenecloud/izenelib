#include <boost/test/unit_test.hpp>
#include <util/ustring/UString.h>
#include <algorithm>

#define scoped_alloc NULL_ALLOCATOR

using namespace std;
using namespace izenelib::util;;
using namespace boost::unit_test;
using namespace boost;

BOOST_AUTO_TEST_SUITE(FindChar_test)

BOOST_AUTO_TEST_CASE(Empty_test)
{
    UString ustr;
    BOOST_CHECK_EQUAL(UString::npos, ustr.find('a'));
}

BOOST_AUTO_TEST_CASE(First_test)
{
    UString ustr("123", UString::UTF_8);
    BOOST_CHECK_EQUAL(0U, ustr.find('1'));
}
BOOST_AUTO_TEST_CASE(Middle_test)
{
    UString ustr("123", UString::UTF_8);
    BOOST_CHECK_EQUAL(1U, ustr.find('2'));
}
BOOST_AUTO_TEST_CASE(Last_test)
{
    UString ustr("123", UString::UTF_8);
    BOOST_CHECK_EQUAL(2U, ustr.find('3'));
}
BOOST_AUTO_TEST_CASE(NotFound_test)
{
    UString ustr("123", UString::UTF_8);
    BOOST_CHECK_EQUAL(UString::npos, ustr.find('0'));
    BOOST_CHECK_EQUAL(UString::npos, ustr.find('4'));
}

BOOST_AUTO_TEST_CASE(BeforePos_test)
{
    UString ustr("123", UString::UTF_8);
    BOOST_CHECK_EQUAL(UString::npos, ustr.find('2', 2));
}

BOOST_AUTO_TEST_CASE(AfterPos_test)
{
    UString ustr("123", UString::UTF_8);
    BOOST_CHECK_EQUAL(1U, ustr.find('2', 0));
}

BOOST_AUTO_TEST_CASE(AtPos_test)
{
    UString ustr("123", UString::UTF_8);
    BOOST_CHECK_EQUAL(1U, ustr.find('2', 1));
}

BOOST_AUTO_TEST_SUITE_END() // Find_test

#undef scoped_alloc
