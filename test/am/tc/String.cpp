#include <boost/test/unit_test.hpp>

#include <am/tc/String.h>

#include <iostream>
#include <algorithm>

using namespace boost::unit_test;
using namespace izenelib::am::tc;

BOOST_AUTO_TEST_SUITE(tc_String_test)

BOOST_AUTO_TEST_CASE(DefaultConstructor_test)
{
    String str;
    BOOST_CHECK(str.empty());
}

BOOST_AUTO_TEST_CASE(SizeConstructor_test)
{
    String str(100);
    BOOST_CHECK(str.empty());
}

BOOST_AUTO_TEST_CASE(CStrConstroctor_test)
{
    char s[] = "123";
    String str(s);

    BOOST_CHECK(str.size() == 3);
    BOOST_CHECK(str[0] == '1');
    BOOST_CHECK(str[1] == '2');
    BOOST_CHECK(str[2] == '3');
}

BOOST_AUTO_TEST_CASE(CStrSizeConstructor_test)
{
    char s[] = "123";
    String str(s, 2);

    BOOST_CHECK(str.size() == 2);
    BOOST_CHECK(str[0] == '1');
    BOOST_CHECK(str[1] == '2');
}

BOOST_AUTO_TEST_CASE(CopyConstructor_test)
{
    char s[] = "123";
    String a(s);
    String b(a);

    BOOST_CHECK(b.data() != a.data());
    BOOST_CHECK(b.size() == 3);
    BOOST_CHECK(b[0] == '1');
    BOOST_CHECK(b[1] == '2');
    BOOST_CHECK(b[2] == '3');
}

BOOST_AUTO_TEST_CASE(Assignment_test)
{
    char s[] = "123";

    String a(s);
    String b;
    b = a;

    BOOST_CHECK(b.data() != a.data());
    BOOST_CHECK(b.size() == 3);
    BOOST_CHECK(b[0] == '1');
    BOOST_CHECK(b[1] == '2');
    BOOST_CHECK(b[2] == '3');
}

BOOST_AUTO_TEST_CASE(AssignmentSelf_test)
{
    char str[] = "123";
    String a(str);
    const char* remember = a.data();
    a = a;

    BOOST_CHECK(remember == a.data());
    BOOST_CHECK(a.size() == 3);
    BOOST_CHECK(a[0] == '1');
    BOOST_CHECK(a[1] == '2');
    BOOST_CHECK(a[2] == '3');
}

BOOST_AUTO_TEST_CASE(Swap_test)
{
    char str[] = "123";
    String a(str);
    const char* remember = a.data();
    String b;
    a.swap(b);

    BOOST_CHECK(a.empty());
    BOOST_CHECK(b.size() == 3);
    BOOST_CHECK(b[0] == '1');
    BOOST_CHECK(b[1] == '2');
    BOOST_CHECK(b[2] == '3');
    BOOST_CHECK(remember == b.data());
}

BOOST_AUTO_TEST_CASE(EmptyIterator_test)
{
    String str;
    BOOST_CHECK(str.begin() == str.end());
}

BOOST_AUTO_TEST_CASE(EmptyReverseIterator_test)
{
    String str;
    BOOST_CHECK(str.rbegin() == str.rend());
}

BOOST_AUTO_TEST_CASE(EmptyConstIterator_test)
{
    const String str;
    BOOST_CHECK(str.begin() == str.end());
}

BOOST_AUTO_TEST_CASE(EmptyConstReverseIterator_test)
{
    const String str;
    BOOST_CHECK(str.rbegin() == str.rend());
}

BOOST_AUTO_TEST_CASE(Iterator_test)
{
    char s[] = "123";
    String str(s);

    BOOST_CHECK(str.begin() != str.end());
    String::iterator i = str.begin();
    BOOST_CHECK(*i++ == '1');
    BOOST_CHECK(i != str.end());
    BOOST_CHECK(*i++ == '2');
    BOOST_CHECK(i != str.end());
    BOOST_CHECK(*i++ == '3');
    BOOST_CHECK(i == str.end());
}

BOOST_AUTO_TEST_CASE(ConstIterator_test)
{
    char s[] = "123";
    const String str(s);

    BOOST_CHECK(str.begin() != str.end());
    String::const_iterator i = str.begin();
    BOOST_CHECK(*i++ == '1');
    BOOST_CHECK(i != str.end());
    BOOST_CHECK(*i++ == '2');
    BOOST_CHECK(i != str.end());
    BOOST_CHECK(*i++ == '3');
    BOOST_CHECK(i == str.end());
}

BOOST_AUTO_TEST_CASE(ReverseIterator_test)
{
    char s[] = "123";
    String str(s);

    BOOST_CHECK(str.rbegin() != str.rend());
    String::reverse_iterator i = str.rbegin();
    BOOST_CHECK(*i++ == '3');
    BOOST_CHECK(i != str.rend());
    BOOST_CHECK(*i++ == '2');
    BOOST_CHECK(i != str.rend());
    BOOST_CHECK(*i++ == '1');
    BOOST_CHECK(i == str.rend());
}

BOOST_AUTO_TEST_CASE(ConstReverseIterator_test)
{
    char s[] = "123";
    const String str(s);

    BOOST_CHECK(str.rbegin() != str.rend());
    String::const_reverse_iterator i = str.rbegin();
    BOOST_CHECK(*i++ == '3');
    BOOST_CHECK(i != str.rend());
    BOOST_CHECK(*i++ == '2');
    BOOST_CHECK(i != str.rend());
    BOOST_CHECK(*i++ == '1');
    BOOST_CHECK(i == str.rend());
}

BOOST_AUTO_TEST_CASE(AtRead_test)
{
    char s[] = "123";
    String str(s);

    BOOST_CHECK(str.at(0) == '1');
    BOOST_CHECK(str.at(1) == '2');
    BOOST_CHECK(str.at(2) == '3');
}

BOOST_AUTO_TEST_CASE(EmptyAtRangeCheck_test)
{
    String b;
    BOOST_CHECK_THROW(b.at(1), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(AtRangeCheck_test)
{
    char s[] = "123";
    String b(s);
    BOOST_CHECK_THROW(b.at(3), std::out_of_range);
    BOOST_CHECK_NO_THROW(b.at(2));
}

BOOST_AUTO_TEST_CASE(AppendString_test)
{
    char s1[] = "123";
    char s2[] = "456";

    String str1(s1);
    String str2(s2);

    str1.append(str2);

    BOOST_CHECK(str1.size() == 6);
    BOOST_CHECK(std::equal(str1.begin(), str1.begin() + 3, s1));
    BOOST_CHECK(std::equal(str1.begin() + 3, str1.end(), s2));
}

BOOST_AUTO_TEST_CASE(AppendCStr_test)
{
    char s1[] = "123";
    char s2[] = "456";

    String str1(s1);

    str1.append(s2);

    BOOST_CHECK(str1.size() == 6);
    BOOST_CHECK(std::equal(str1.begin(), str1.begin() + 3, s1));
    BOOST_CHECK(std::equal(str1.begin() + 3, str1.end(), s2));
}

BOOST_AUTO_TEST_CASE(AppendCStrSize_test)
{
    char s1[] = "123";
    char s2[] = "456";

    String str1(s1);

    str1.append(s2, 2);

    BOOST_CHECK(str1.size() == 5);
    BOOST_CHECK(std::equal(str1.begin(), str1.begin() + 3, s1));
    BOOST_CHECK(std::equal(str1.begin() + 3, str1.end(), s2));
}

BOOST_AUTO_TEST_CASE(StringPlusString_test)
{
    char s1[] = "123";
    char s2[] = "456";

    String str1(s1);
    String str2(s2);

    String str = str1 + str2;

    BOOST_CHECK(str.size() == 6);
    BOOST_CHECK(std::equal(str.begin(), str.begin() + 3, s1));
    BOOST_CHECK(std::equal(str.begin() + 3, str.end(), s2));
}

BOOST_AUTO_TEST_CASE(StringPlusCStr_test)
{
    char s1[] = "123";
    char s2[] = "456";

    String str1(s1);

    String str = str1 + s2;

    BOOST_CHECK(str.size() == 6);
    BOOST_CHECK(std::equal(str.begin(), str.begin() + 3, s1));
    BOOST_CHECK(std::equal(str.begin() + 3, str.end(), s2));
}

BOOST_AUTO_TEST_CASE(CStrPlusString_test)
{
    char s1[] = "123";
    char s2[] = "456";

    String str2(s2);

    String str = s1 + str2;

    BOOST_CHECK(str.size() == 6);
    BOOST_CHECK(std::equal(str.begin(), str.begin() + 3, s1));
    BOOST_CHECK(std::equal(str.begin() + 3, str.end(), s2));
}

BOOST_AUTO_TEST_CASE(Equal_test)
{
    char s1[] = "123";
    char s2[] = "123";

    String str1(s1);
    String str2(s2);

    BOOST_CHECK(str1 == str2);
}

BOOST_AUTO_TEST_CASE(Unequal_test)
{
    char s1[] = "123";
    char s2[] = "103";

    String str1(s1);
    String str2(s2);

    BOOST_CHECK(str1 != str2);
}

BOOST_AUTO_TEST_CASE(Less_test)
{
    String str1("123");
    String str2("133");

    BOOST_CHECK(str1 < str2);
}

BOOST_AUTO_TEST_CASE(ShorterIsLess_test)
{
    String str1("123");
    String str2("1234");

    BOOST_CHECK(str1 < str2);
}

BOOST_AUTO_TEST_CASE(EqualIsNotLess_test)
{
    String str1("123");
    String str2("123");

    BOOST_CHECK(!(str1 < str2));
}

BOOST_AUTO_TEST_CASE(LargerIsNotLess_test)
{
    String str1("133");
    String str2("123");

    BOOST_CHECK(!(str1 < str2));
}

BOOST_AUTO_TEST_CASE(ToBuffer_test)
{
    String str("123");
    ::izenelib::am::raw::Buffer buf;
    str.toBuffer(buf);

    BOOST_CHECK(str.empty());
    BOOST_CHECK(buf.size() == 3);
    BOOST_CHECK(!buf.empty());
    BOOST_CHECK(buf[0] == '1');
    BOOST_CHECK(buf[1] == '2');
    BOOST_CHECK(buf[2] == '3');
}

BOOST_AUTO_TEST_SUITE_END()
