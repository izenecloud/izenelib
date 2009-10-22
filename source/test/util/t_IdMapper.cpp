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

BOOST_FIXTURE_TEST_SUITE(IdMapper_test, MapFixture)

BOOST_AUTO_TEST_CASE(Empty_FindId)
{
    testNonExist("whatever");
}

BOOST_AUTO_TEST_CASE(Empty_FindValue)
{
    testNonExist(0);
}

BOOST_AUTO_TEST_CASE(InsertOne)
{
    std::size_t id = map.insert("one");

    testExist(id, "one");
}

BOOST_AUTO_TEST_CASE(InsertOne_FindAnother)
{
    std::size_t id = map.insert("one");

    testNonExist("two");
    testNonExist(id + 1);
}

BOOST_AUTO_TEST_CASE(InsertOneWithId)
{
    BOOST_CHECK(map.insert("ten", 10));

    testExist(10, "ten");
}

BOOST_AUTO_TEST_CASE(InsertExist_test)
{
    std::size_t id = map.insert("one");
    BOOST_ASSERT(map.insert("one") == id);

    testExist(id, "one");
}

BOOST_AUTO_TEST_CASE(InsertExistWithId_test)
{
    std::size_t id = map.insert("one");
    BOOST_ASSERT(!map.insert("one", 10));

    testExist(id, "one");
}

BOOST_AUTO_TEST_CASE(InsertThree_test)
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

BOOST_AUTO_TEST_SUITE_END()
