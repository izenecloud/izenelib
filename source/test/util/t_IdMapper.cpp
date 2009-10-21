/**
 * @file test/util/t_IdMapper.cpp
 */
#define BOOST_TEST_MODULE IdMapper
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <util/IdMapper.h>
#include <string>

using namespace izenelib::util;

namespace { // {anonymous}
struct MapFixture
{
    IdMapper<std::string> map;

    void testExist(std::size_t id, const std::string& str)
    {
        const std::size_t* readId = map.findIdByValue(str);
        BOOST_REQUIRE(readId);
        BOOST_CHECK(*readId == id);

        const std::string* readStr = map.findValueById(id);
        BOOST_REQUIRE(readStr);
        BOOST_CHECK(*readStr == str);
    }

    void testNonExist(std::size_t id)
    {
        BOOST_CHECK(0 == map.findValueById(id));
    }

    void testNonExist(const std::string& str)
    {
        BOOST_CHECK(0 == map.findIdByValue(str));
    }
};
} // namespace {anonymous}


BOOST_FIXTURE_TEST_CASE(Empty_FindId, MapFixture)
{
    testNonExist("whatever");
}

BOOST_FIXTURE_TEST_CASE(Empty_FindValue, MapFixture)
{
    testNonExist(0);
}

BOOST_FIXTURE_TEST_CASE(InsertOne, MapFixture)
{
    std::size_t id = map.insert("one");

    testExist(id, "one");
}

BOOST_FIXTURE_TEST_CASE(InsertOne_FindAnother, MapFixture)
{
    std::size_t id = map.insert("one");

    testNonExist("two");
    testNonExist(id + 1);
}

BOOST_FIXTURE_TEST_CASE(InsertOneWithId, MapFixture)
{
    BOOST_CHECK(map.insert("ten", 10));

    testExist(10, "ten");
}

BOOST_FIXTURE_TEST_CASE(InsertExist_test, MapFixture)
{
    std::size_t id = map.insert("one");
    BOOST_ASSERT(map.insert("one") == id);

    testExist(id, "one");
}

BOOST_FIXTURE_TEST_CASE(InsertExistWithId_test, MapFixture)
{
    std::size_t id = map.insert("one");
    BOOST_ASSERT(!map.insert("one", 10));

    testExist(id, "one");
}

BOOST_FIXTURE_TEST_CASE(InsertThree_test, MapFixture)
{
    std::size_t idOne = map.insert("one");
    std::size_t idTwo = map.insert("two");
    std::size_t idThree = map.insert("three");

    BOOST_CHECK(idOne != idTwo);
    BOOST_CHECK(idTwo != idThree);
    BOOST_CHECK(idOne != idThree);

    testExist(idOne, "one");
    testExist(idTwo, "two");
    testExist(idThree, "three");

    std::size_t minId = idOne;
    std::size_t maxId = idOne;
    if (idTwo < minId)
    {
        minId = idTwo;
    }
    else if (idTwo > maxId)
    {
        maxId = idTwo;
    }
    if (idThree < minId)
    {
        minId = idThree;
    }
    else if (idThree > maxId)
    {
        maxId = idThree;
    }

    if (minId > 0)
    {
        testNonExist(minId - 1);
    }
    testNonExist(maxId + 1);
}
