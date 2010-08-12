#include <boost/test/unit_test.hpp>
#include <util/ustring/UString.h>
#include <algorithm>
#include <vector>
#include <cstring>
#include <string>

#define scoped_alloc NULL_ALLOCATOR

using namespace std;
using namespace izenelib::util;;
using namespace boost::unit_test;
using namespace boost;

BOOST_AUTO_TEST_SUITE(FindStr_test)

BOOST_AUTO_TEST_CASE(Empty_test)
{
    UString ustr;
    UString needle;
    BOOST_CHECK_EQUAL(UString::npos, ustr.find(needle));
}

BOOST_AUTO_TEST_CASE(EmptyTarget_test)
{
    UString ustr;
    UString needle("me", UString::UTF_8);
    BOOST_CHECK_EQUAL(UString::npos, ustr.find(needle));
}

BOOST_AUTO_TEST_CASE(EmptyNeedle_test)
{
    UString ustr("abcdefg", UString::UTF_8);
    UString needle;
    BOOST_CHECK_EQUAL(UString::npos, ustr.find(needle));
}

BOOST_AUTO_TEST_CASE(First_test)
{
    UString ustr("abcdefg", UString::UTF_8);
    UString needle("abc", UString::UTF_8);
    BOOST_CHECK_EQUAL(0U, ustr.find(needle));
}

BOOST_AUTO_TEST_CASE(Middle_test)
{
    UString ustr("abcdefg", UString::UTF_8);
    UString needle("cde", UString::UTF_8);
    BOOST_CHECK_EQUAL(2U, ustr.find(needle));
}

BOOST_AUTO_TEST_CASE(Last_test)
{
    UString ustr("abcdefg", UString::UTF_8);
    UString needle("efg", UString::UTF_8);
    BOOST_CHECK_EQUAL(4U, ustr.find(needle));
}

BOOST_AUTO_TEST_CASE(BeforePos_test)
{
    UString ustr("abcdefg", UString::UTF_8);
    UString needle("cde", UString::UTF_8);
    BOOST_CHECK_EQUAL(2U, ustr.find(needle, 1));
}

BOOST_AUTO_TEST_CASE(AtPos_test)
{
    UString ustr("abcdefg", UString::UTF_8);
    UString needle("cde", UString::UTF_8);
    BOOST_CHECK_EQUAL(2U, ustr.find(needle, 2));
}

BOOST_AUTO_TEST_CASE(InPos_test)
{
    UString ustr("abcdefg", UString::UTF_8);
    UString needle("cde", UString::UTF_8);
    BOOST_CHECK_EQUAL(UString::npos, ustr.find(needle, 3));
}

BOOST_AUTO_TEST_CASE(AfterPos_test)
{
    UString ustr("abcdefg", UString::UTF_8);
    UString needle("cde", UString::UTF_8);
    BOOST_CHECK_EQUAL(UString::npos, ustr.find(needle, 5));
}

BOOST_AUTO_TEST_SUITE_END() // FindStr_test


namespace { // {anonymous}
struct CharConversionFixture
{
    typedef UString::value_type char_type;

    CharConversionFixture()
    : needle_(1, 0)
    {}

    void initNeedle(const char* str)
    {
        initNeedle(str, strlen(str));
    }

    void initNeedle(const char* str, size_t len)
    {
        needle_.resize(len + 1);
        needle_.back() = 0;
        copy(str, str + len, needle_.begin());
    }

    const char_type* needle() const
    {
        return &needle_.front();
    }

    const size_t size() const
    {
        return needle_.size() - 1;
    }

private:
    std::vector<char_type> needle_;
};

} // namespace {anonymous}

// C style, array ending with 0

BOOST_FIXTURE_TEST_SUITE(FindCStr_test, CharConversionFixture)

BOOST_AUTO_TEST_CASE(Empty_test)
{
    UString ustr;
    BOOST_CHECK_EQUAL(UString::npos, ustr.find(needle()));
}

BOOST_AUTO_TEST_CASE(EmptyTarget_test)
{
    UString ustr;
    initNeedle("me");
    BOOST_CHECK_EQUAL(UString::npos, ustr.find(needle()));
}

BOOST_AUTO_TEST_CASE(EmptyNeedle_test)
{
    UString ustr("abcdefg", UString::UTF_8);
    BOOST_CHECK_EQUAL(UString::npos, ustr.find(needle()));
}

BOOST_AUTO_TEST_CASE(First_test)
{
    UString ustr("abcdefg", UString::UTF_8);
    initNeedle("abc");
    BOOST_CHECK_EQUAL(0U, ustr.find(needle()));
}

BOOST_AUTO_TEST_CASE(Middle_test)
{
    UString ustr("abcdefg", UString::UTF_8);
    initNeedle("cde");
    BOOST_CHECK_EQUAL(2U, ustr.find(needle()));
}

BOOST_AUTO_TEST_CASE(Last_test)
{
    UString ustr("abcdefg", UString::UTF_8);
    initNeedle("efg");
    BOOST_CHECK_EQUAL(4U, ustr.find(needle()));
}

BOOST_AUTO_TEST_CASE(BeforePos_test)
{
    UString ustr("abcdefg", UString::UTF_8);
    initNeedle("cde");
    BOOST_CHECK_EQUAL(2U, ustr.find(needle(), 1));
}

BOOST_AUTO_TEST_CASE(AtPos_test)
{
    UString ustr("abcdefg", UString::UTF_8);
    initNeedle("cde");
    BOOST_CHECK_EQUAL(2U, ustr.find(needle(), 2));
}

BOOST_AUTO_TEST_CASE(InPos_test)
{
    UString ustr("abcdefg", UString::UTF_8);
    initNeedle("cde");
    BOOST_CHECK_EQUAL(UString::npos, ustr.find(needle(), 3));
}

BOOST_AUTO_TEST_CASE(AfterPos_test)
{
    UString ustr("abcdefg", UString::UTF_8);
    initNeedle("cde");
    BOOST_CHECK_EQUAL(UString::npos, ustr.find(needle(), 5));
}

BOOST_AUTO_TEST_SUITE_END() // FindCStr_test

BOOST_FIXTURE_TEST_SUITE(SearchZero_test, CharConversionFixture)

BOOST_AUTO_TEST_CASE(Middle_test)
{
    UString ustr(string("abc\0efg", 7), UString::ISO8859_15);
    initNeedle("c\0e", 3);

    BOOST_CHECK_EQUAL(2U, ustr.find(needle(), 0, size()));
}

BOOST_AUTO_TEST_CASE(Prefix_test)
{
    UString ustr(string("cbc\0efg", 7), UString::ISO8859_15);
    initNeedle("c\0e", 3);

    BOOST_CHECK_EQUAL(2U, ustr.find(needle(), 0, size()));
}

BOOST_AUTO_TEST_CASE(UstringMiddle_test)
{
    UString ustr(string("abc\0efg", 7), UString::ISO8859_15);
    UString needle(string("c\0e", 3), UString::ISO8859_15);

    BOOST_CHECK_EQUAL(2U, ustr.find(needle));
}

BOOST_AUTO_TEST_CASE(UstringPrefix_test)
{
    UString ustr(string("abc\0efg", 7), UString::ISO8859_15);
    UString needle(string("c\0e", 3), UString::ISO8859_15);

    BOOST_CHECK_EQUAL(2U, ustr.find(needle));
}

BOOST_AUTO_TEST_SUITE_END() // SearchZero_test

#undef scoped_alloc
