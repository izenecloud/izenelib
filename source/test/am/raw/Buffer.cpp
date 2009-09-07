#include <boost/test/unit_test.hpp>

#include <am/raw/Buffer.h>

#include <iostream>

using namespace boost::unit_test;
using namespace izenelib::am::raw;

namespace { // anonymous
struct Fixture
{
    Fixture()
    {
        deleterCounter = 0;
    }

    static void testDeleter(void* p)
    {
        ++deleterCounter;
    }

    static unsigned deleterCounter;
};
unsigned Fixture::deleterCounter = 0;
} // end of anonymous namespace

BOOST_FIXTURE_TEST_SUITE(raw_Buffer_test, Fixture)

BOOST_AUTO_TEST_CASE(DefaultConstructor_test)
{
    Buffer buf;
    BOOST_CHECK(buf.empty());
}

BOOST_AUTO_TEST_CASE(SizeConstructor_test)
{
    Buffer buf(100);
    BOOST_CHECK(!buf.empty());
    BOOST_CHECK(buf.size() == 100);
}

BOOST_AUTO_TEST_CASE(AttachConstroctor_test)
{
    char str[] = "123";
    Buffer buf(str, 3);

    BOOST_CHECK(buf.size() == 3);
    BOOST_CHECK(buf[0] == '1');
    BOOST_CHECK(buf[1] == '2');
    BOOST_CHECK(buf[2] == '3');
}

BOOST_AUTO_TEST_CASE(Attach_test)
{
    Buffer buf;
    char str[] = "123";

    buf.attach(str, 3);

    BOOST_CHECK(buf.size() == 3);
    BOOST_CHECK(buf[0] == '1');
    BOOST_CHECK(buf[1] == '2');
    BOOST_CHECK(buf[2] == '3');
}

BOOST_AUTO_TEST_CASE(AttachDestructFirst_test)
{
    char str[] = "123";
    Buffer a(str, 3, testDeleter);

    char str2[] = "456";
    a.attach(str2, 3);
    BOOST_CHECK(deleterCounter == 1U);
}

BOOST_AUTO_TEST_CASE(AttachSameNotDestruct_test)
{
    char str[] = "123";
    Buffer a(str, 3, testDeleter);

    a.attach(str, 3);
    BOOST_CHECK(deleterCounter == 0U);
}

BOOST_AUTO_TEST_CASE(CopyAttach_test)
{
    Buffer buf;
    char str[] = "123";

    buf.copyAttach(str, 3);

    BOOST_CHECK(buf.data() != str);
    BOOST_CHECK(buf.size() == 3);
    BOOST_CHECK(buf[0] == '1');
    BOOST_CHECK(buf[1] == '2');
    BOOST_CHECK(buf[2] == '3');
}

BOOST_AUTO_TEST_CASE(CopyAttachDestructFirst_test)
{
    char str[] = "123";
    Buffer a(str, 3, testDeleter);

    a.copyAttach("456", 3);
    BOOST_CHECK(deleterCounter == 1);
}

BOOST_AUTO_TEST_CASE(CopyAttachSameNotDestruct_test)
{
    char str[] = "123";
    Buffer a(str, 3, testDeleter);

    a.copyAttach(str, 3);
    BOOST_CHECK(deleterCounter == 0);
}

BOOST_AUTO_TEST_CASE(Deleter_test)
{
    char str[] = "123";

    {
        Buffer buf;
        buf.attach(str, 3, testDeleter);
    }

    BOOST_CHECK_EQUAL(deleterCounter, 1U);
}

BOOST_AUTO_TEST_CASE(CopyConstructor_test)
{
    char str[] = "123";

    Buffer a(str, 3);

    Buffer b(a);

    BOOST_CHECK(b.data() != a.data());
    BOOST_CHECK(b.size() == 3);
    BOOST_CHECK(b[0] == '1');
    BOOST_CHECK(b[1] == '2');
    BOOST_CHECK(b[2] == '3');
}


BOOST_AUTO_TEST_CASE(CopyConstructorNotCopyDeleter_test)
{
    {
        char str[] = "123";
        Buffer a(str, 3, testDeleter);
        Buffer b(a);
    }

    BOOST_CHECK(deleterCounter == 1);
}

BOOST_AUTO_TEST_CASE(Assignment_test)
{
    char str[] = "123";

    Buffer a(str, 3);

    Buffer b;

    b = a;

    BOOST_CHECK(b.data() != a.data());
    BOOST_CHECK(b.size() == 3);
    BOOST_CHECK(b[0] == '1');
    BOOST_CHECK(b[1] == '2');
    BOOST_CHECK(b[2] == '3');
}

BOOST_AUTO_TEST_CASE(AssignmentNotCopyDeleter_test)
{
    {
        char str[] = "123";
        Buffer a(str, 3, testDeleter);
        Buffer b;
        b = a;
    }

    BOOST_CHECK_EQUAL(deleterCounter, 1U);
}

BOOST_AUTO_TEST_CASE(AssignmentSelf_test)
{
    {
        char str[] = "123";
        Buffer a(str, 3, testDeleter);
        a = a;

        BOOST_CHECK(a.data() == str);
        BOOST_CHECK(a.size() == 3);
        BOOST_CHECK(a[0] == '1');
        BOOST_CHECK(a[1] == '2');
        BOOST_CHECK(a[2] == '3');
    }

    BOOST_CHECK_EQUAL(deleterCounter, 1U);
}

BOOST_AUTO_TEST_CASE(AssignmentDestructFirst_test)
{
    char str[] = "123";
    Buffer a(str, 3, testDeleter);
    Buffer b;
    a = b;

    BOOST_CHECK(a.empty());

    BOOST_CHECK_EQUAL(deleterCounter, 1U);
}

BOOST_AUTO_TEST_CASE(Swap_test)
{
    {
        char str[] = "123";
        Buffer a(str, 3, testDeleter);
        Buffer b;
        a.swap(b);
        BOOST_CHECK_EQUAL(deleterCounter, 0U);

        BOOST_CHECK(a.empty());
        BOOST_CHECK(b.size() == 3);
        BOOST_CHECK(b[0] == '1');
        BOOST_CHECK(b[1] == '2');
        BOOST_CHECK(b[2] == '3');
        BOOST_CHECK(b.data() == str);
    }

    BOOST_CHECK_EQUAL(deleterCounter, 1U);
}

BOOST_AUTO_TEST_CASE(EmptyIterator_test)
{
    Buffer buf;
    BOOST_CHECK(buf.begin() == buf.end());
}

BOOST_AUTO_TEST_CASE(EmptyReverseIterator_test)
{
    Buffer buf;
    BOOST_CHECK(buf.rbegin() == buf.rend());
}

BOOST_AUTO_TEST_CASE(EmptyConstIterator_test)
{
    const Buffer buf;
    BOOST_CHECK(buf.begin() == buf.end());
}

BOOST_AUTO_TEST_CASE(EmptyConstReverseIterator_test)
{
    const Buffer buf;
    BOOST_CHECK(buf.rbegin() == buf.rend());
}

BOOST_AUTO_TEST_CASE(Iterator_test)
{
    char str[] = "123";
    Buffer buf(str, 3);

    BOOST_CHECK(buf.begin() != buf.end());
    Buffer::iterator i = buf.begin();
    BOOST_CHECK(*i++ == '1');
    BOOST_CHECK(i != buf.end());
    BOOST_CHECK(*i++ == '2');
    BOOST_CHECK(i != buf.end());
    BOOST_CHECK(*i++ == '3');
    BOOST_CHECK(i == buf.end());
}

BOOST_AUTO_TEST_CASE(ConstIterator_test)
{
    char str[] = "123";
    const Buffer buf(str, 3);

    BOOST_CHECK(buf.begin() != buf.end());
    Buffer::const_iterator i = buf.begin();
    BOOST_CHECK(*i++ == '1');
    BOOST_CHECK(i != buf.end());
    BOOST_CHECK(*i++ == '2');
    BOOST_CHECK(i != buf.end());
    BOOST_CHECK(*i++ == '3');
    BOOST_CHECK(i == buf.end());
}

BOOST_AUTO_TEST_CASE(ReverseIterator_test)
{
    char str[] = "123";
    Buffer buf(str, 3);

    BOOST_CHECK(buf.rbegin() != buf.rend());
    Buffer::reverse_iterator i = buf.rbegin();
    BOOST_CHECK(*i++ == '3');
    BOOST_CHECK(i != buf.rend());
    BOOST_CHECK(*i++ == '2');
    BOOST_CHECK(i != buf.rend());
    BOOST_CHECK(*i++ == '1');
    BOOST_CHECK(i == buf.rend());
}

BOOST_AUTO_TEST_CASE(ConstReverseIterator_test)
{
    char str[] = "123";
    const Buffer buf(str, 3);

    BOOST_CHECK(buf.rbegin() != buf.rend());
    Buffer::const_reverse_iterator i = buf.rbegin();
    BOOST_CHECK(*i++ == '3');
    BOOST_CHECK(i != buf.rend());
    BOOST_CHECK(*i++ == '2');
    BOOST_CHECK(i != buf.rend());
    BOOST_CHECK(*i++ == '1');
    BOOST_CHECK(i == buf.rend());
}

BOOST_AUTO_TEST_CASE(FrontRead_test)
{
    char str[] = "123";
    const Buffer buf(str, 3);

    BOOST_CHECK(buf.front() == '1');
}

BOOST_AUTO_TEST_CASE(FrontWrite_test)
{
    char str[] = "123";
    Buffer buf(str, 3);

    buf.front() = '0';

    BOOST_CHECK(str[0] == '0');
}

BOOST_AUTO_TEST_CASE(BackRead_test)
{
    char str[] = "123";
    const Buffer buf(str, 3);

    BOOST_CHECK(buf.back() == '3');
}

BOOST_AUTO_TEST_CASE(BackWrite_test)
{
    char str[] = "123";
    Buffer buf(str, 3);

    buf.back() = '0';

    BOOST_CHECK(str[2] == '0');
}

BOOST_AUTO_TEST_CASE(IndexWrite_test)
{
    char str[] = "123";
    Buffer buf(str, 3);

    buf[0] = 'a';
    buf[1] = 'b';
    buf[2] = 'c';

    BOOST_CHECK(str[0] == 'a');
    BOOST_CHECK(str[1] == 'b');
    BOOST_CHECK(str[2] == 'c');
}

BOOST_AUTO_TEST_CASE(AtRead_test)
{
    char str[] = "123";
    Buffer buf(str, 3);

    BOOST_CHECK(buf.at(0) == '1');
    BOOST_CHECK(buf.at(1) == '2');
    BOOST_CHECK(buf.at(2) == '3');
}

BOOST_AUTO_TEST_CASE(AtWrite_test)
{
    char str[] = "123";
    Buffer buf(str, 3);

    buf.at(0) = 'a';
    buf.at(1) = 'b';
    buf.at(2) = 'c';

    BOOST_CHECK(str[0] == 'a');
    BOOST_CHECK(str[1] == 'b');
    BOOST_CHECK(str[2] == 'c');
}

BOOST_AUTO_TEST_CASE(EmptyAtRangeCheck_test)
{
    Buffer b;
    BOOST_CHECK_THROW(b.at(1), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(AtRangeCheck_test)
{
    char str[] = "123";
    Buffer b(str, 3);
    BOOST_CHECK_THROW(b.at(3), std::out_of_range);
    BOOST_CHECK_NO_THROW(b.at(2));
}

BOOST_AUTO_TEST_SUITE_END()
