#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>

#include <cstdio>

#include <am/tc/raw/Hash.h>
#include <am/tc/raw/BTree.h>

#define DIR_PREFIX "./tmp/am_tc_raw_Db_"

using namespace boost::unit_test;
using namespace izenelib::am::tc::raw;

typedef boost::mpl::list<Hash, BTree> test_types;

BOOST_AUTO_TEST_SUITE(tc_raw_Db_test)

BOOST_AUTO_TEST_CASE_TEMPLATE(DefaultConstructor_test, T, test_types)
{
    T h;
    BOOST_CHECK(!h.isOpened());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(FileConstructor_test, T, test_types)
{
    T h(DIR_PREFIX "FileConstructor_test");
    BOOST_CHECK(!h.isOpened());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Open_test, T, test_types)
{
    T h(DIR_PREFIX "Open_test");
    bool opened;
    BOOST_WARN(opened = h.open());

    if (opened)
    {
        BOOST_CHECK(h.isOpened());
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(OpenWithFile_test, T, test_types)
{
    T h;
    bool opened;
    BOOST_WARN(opened = h.open(DIR_PREFIX "OpenWithFile_test"));

    if (opened)
    {
        BOOST_CHECK(h.isOpened());
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Close_test, T, test_types)
{
    T h(DIR_PREFIX "Close_test");
    BOOST_WARN(h.open());

    h.close();
    BOOST_CHECK(!h.isOpened());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(CloseWithoutOpen_test, T, test_types)
{
    T h(DIR_PREFIX "Close_test");
    h.close();
    BOOST_CHECK(!h.isOpened());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(NotOpenedIsEmpty_test, T, test_types)
{
    T h(DIR_PREFIX "NotOpenedIsEmpty_test");
    BOOST_CHECK(h.empty());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(NewIsEmpty_test, T, test_types)
{
    T h(DIR_PREFIX "NewIsEmpty_test");
    h.open();
    BOOST_CHECK(h.empty());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(InsertGet_test, T, test_types)
{
    std::remove(DIR_PREFIX "InsertGet_test");
    T h(DIR_PREFIX "InsertGet_test");
    BOOST_CHECK(h.open());

    char str1[] = "abc";
    char str2[] = "def";
    Buffer a(str1, 3);
    Buffer b(str2, 3);
    BOOST_CHECK(h.insert(a, b));
    Buffer c;
    BOOST_CHECK(h.get(a, c));
    BOOST_CHECK(c.size() == 3);
    BOOST_CHECK(c[0] == 'd');
    BOOST_CHECK(c[1] == 'e');
    BOOST_CHECK(c[2] == 'f');
}

BOOST_AUTO_TEST_CASE_TEMPLATE(GetFromEmtpy_test, T, test_types)
{
    std::remove(DIR_PREFIX "GetFromEmtpy_test");
    T h(DIR_PREFIX "GetFromEmtpy_test");
    BOOST_CHECK(h.open());

    char str1[] = "abc";
    Buffer a(str1, 3);
    Buffer c;
    BOOST_CHECK(!(h.get(a, c)));
    BOOST_CHECK(c.empty());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(GetNotExisted_test, T, test_types)
{
    std::remove(DIR_PREFIX "GetNotExisted_test");
    T h(DIR_PREFIX "GetNotExisted_test");
    BOOST_CHECK(h.open());

    char str1[] = "abc";
    char str2[] = "def";

    Buffer a(str1, 3);
    Buffer b(str2, 3);
    BOOST_CHECK(h.insert(a, b));

    char str3[] = "ab";
    Buffer c(str3, 2);
    Buffer d;
    BOOST_CHECK(!h.get(c, d));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(InsertExisted_test, T, test_types)
{
    std::remove(DIR_PREFIX "InsertExisted_test");
    T h(DIR_PREFIX "InsertExisted_test");
    BOOST_CHECK(h.open());

    char str1[] = "abc";
    char str2[] = "def";

    BOOST_CHECK(h.insert(Buffer(str1, 3), Buffer(str2, 3)));

    char str3[] = "abc";
    char str4[] = "ghi";
    BOOST_CHECK(!h.insert(Buffer(str3, 3), Buffer(str4, 3)));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(InsertAndSize_test, T, test_types)
{
    std::remove(DIR_PREFIX "InsertAndSize_test");
    T h(DIR_PREFIX "InsertAndSize_test");
    BOOST_CHECK(h.open());

    {
        char key[] = "1";
        char value[] = "a";
        BOOST_CHECK(h.insert(Buffer(key, 1), Buffer(value, 1)));
        BOOST_CHECK(h.size() == 1);
    }

    {
        char key[] = "2";
        char value[] = "b";
        BOOST_CHECK(h.insert(Buffer(key, 1), Buffer(value, 1)));
        BOOST_CHECK(h.size() == 2);
    }

    {
        char key[] = "3";
        char value[] = "c";
        BOOST_CHECK(h.insert(Buffer(key, 1), Buffer(value, 1)));
        BOOST_CHECK(h.size() == 3);
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(UpdateAsInsert_test, T, test_types)
{
    std::remove(DIR_PREFIX "UpdateAsInsert_test");
    T h(DIR_PREFIX "UpdateAsInsert_test");
    BOOST_CHECK(h.open());

    char str1[] = "abc";
    char str2[] = "def";

    BOOST_CHECK(h.update(Buffer(str1, 3), Buffer(str2, 3)));
    BOOST_CHECK(h.size() == 1);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Update_test, T, test_types)
{
    std::remove(DIR_PREFIX "Update_test");
    T h(DIR_PREFIX "Update_test");
    BOOST_CHECK(h.open());

    {
        char key[] = "1";
        char value[] = "a";
        BOOST_CHECK(h.insert(Buffer(key, 1), Buffer(value, 1)));
    }

    {
        char key[] = "1";
        char value[] = "b";
        BOOST_CHECK(h.update(Buffer(key, 1), Buffer(value, 1)));
        BOOST_CHECK(h.size() == 1);
    }

    {
        char key[] = "1";
        Buffer value;
        BOOST_CHECK(h.get(Buffer(key, 1), value));
        BOOST_CHECK(value.size() == 1);
        BOOST_CHECK(value[0] == 'b');
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Exist_test, T, test_types)
{
    std::remove(DIR_PREFIX "Exist_test");
    T h(DIR_PREFIX "Exist_test");
    BOOST_CHECK(h.open());

    {
        char key[] = "key";
        char value[] = "value";
        BOOST_CHECK(h.insert(Buffer(key, 3), Buffer(value, 5)));
        BOOST_CHECK(h.exist(Buffer(key, 3)));
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(EmptyNotExist_test, T, test_types)
{
    std::remove(DIR_PREFIX "EmptyNotExist_test");
    T h(DIR_PREFIX "EmptyNotExist_test");
    BOOST_CHECK(h.open());

    char key[] = "key";
    BOOST_CHECK(!h.exist(Buffer(key, 3)));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(NotExist_test, T, test_types)
{
    std::remove(DIR_PREFIX "NotExist_test");
    T h(DIR_PREFIX "NotExist_test");
    BOOST_CHECK(h.open());

    {
        char key[] = "key";
        char value[] = "value";
        BOOST_CHECK(h.insert(Buffer(key, 3), Buffer(value, 5)));
    }

    {
        char key[] = "notexist";
        BOOST_CHECK(!h.exist(Buffer(key, 8)));
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Del_test, T, test_types)
{
    std::remove(DIR_PREFIX "Del_test");
    T h(DIR_PREFIX "Del_test");
    BOOST_CHECK(h.open());

    {
        char key[] = "key";
        char value[] = "value";
        BOOST_CHECK(h.insert(Buffer(key, 3), Buffer(value, 5)));
        BOOST_CHECK(h.size() == 1);
    }

    {
        char key[] = "key";
        BOOST_CHECK(h.del(Buffer(key, 3)));
        BOOST_CHECK(h.empty());
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(DelNotExisted_test, T, test_types)
{
    std::remove(DIR_PREFIX "DelNotExisted_test");
    T h(DIR_PREFIX "DelNotExisted_test");
    BOOST_CHECK(h.open());

    {
        char key[] = "key";
        char value[] = "value";
        BOOST_CHECK(h.insert(Buffer(key, 3), Buffer(value, 5)));
        BOOST_CHECK(h.size() == 1);
    }

    {
        char key[] = "key1";
        BOOST_CHECK(!h.del(Buffer(key, 4)));
        BOOST_CHECK(h.size() == 1);
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Update1In2, T, test_types)
{
    std::remove(DIR_PREFIX "Update1In2_test");
    T h(DIR_PREFIX "Update1In2_test");
    BOOST_CHECK(h.open());

    {
        char key[] = "1";
        char value[] = "a";
        BOOST_CHECK(h.insert(Buffer(key, 1), Buffer(value, 1)));
    }

    {
        char key[] = "2";
        char value[] = "b";
        BOOST_CHECK(h.insert(Buffer(key, 1), Buffer(value, 1)));
    }

    {
        char key[] = "1";
        char value[] = "c";
        BOOST_CHECK(h.update(Buffer(key, 1), Buffer(value, 1)));
    }

    {
        BOOST_CHECK(h.size() == 2);
        Buffer buf;
        Buffer key(1);
        key[0] = '1';
        h.get(key, buf);
        BOOST_CHECK(buf.size() == 1);
        BOOST_CHECK(buf[0] = 'c');
        key[0] = '2';
        h.get(key, buf);
        BOOST_CHECK(buf.size() == 1);
        BOOST_CHECK(buf[0] = 'b');
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Del1In2, T, test_types)
{
    std::remove(DIR_PREFIX "Del1In2_test");
    T h(DIR_PREFIX "Del1In2_test");
    BOOST_CHECK(h.open());

    {
        char key[] = "1";
        char value[] = "a";
        BOOST_CHECK(h.insert(Buffer(key, 1), Buffer(value, 1)));
    }

    {
        char key[] = "2";
        char value[] = "b";
        BOOST_CHECK(h.insert(Buffer(key, 1), Buffer(value, 1)));
    }

    {
        Buffer buf;
        buf.copyAttach("1", 1);
        BOOST_CHECK(h.del(buf));
    }

    {
        Buffer buf;
        buf.copyAttach("1", 1);
        BOOST_CHECK(!h.exist(buf));
        buf.copyAttach("2", 1);
        BOOST_CHECK(h.exist(buf));
    }
}

BOOST_AUTO_TEST_SUITE_END() // tc_raw_Db_test
