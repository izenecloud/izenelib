///
/// @file t_concurrent_set.cpp
/// @brief test functions in ConcurrentSet
/// @author Jun Jiang
/// @date Created 2011-11-24
///

#include <am/matrix/concurrent_set.h>

#include <boost/test/unit_test.hpp>

#include <set>

namespace
{

template<typename SetType>
void checkSet()
{
    typedef typename SetType::key_type KeyType;
    typedef typename SetType::container_type ContainerType;

    SetType set;

    BOOST_TEST_MESSAGE("check empty...");
    BOOST_CHECK_EQUAL(set.size(), 0U);
    BOOST_CHECK(set.empty());
    BOOST_CHECK_EQUAL(set.count(10), 0U);

    BOOST_TEST_MESSAGE("check insert...");
    const int count = 10;
    for (int i=0; i<count; ++i)
    {
        BOOST_CHECK(set.insert(i));
        BOOST_CHECK(set.insert(i) == false);
        BOOST_CHECK_EQUAL(set.count(i), 1U);
    }
    BOOST_CHECK_EQUAL(set.size(), count);
    BOOST_CHECK(set.empty() == false);

    BOOST_TEST_MESSAGE("check erase...");
    const int eraseIndex = 5;
    for (int i=eraseIndex; i<count; ++i)
    {
        BOOST_CHECK(set.erase(i));
        BOOST_CHECK(set.erase(i) == false);
        BOOST_CHECK_EQUAL(set.count(i), 0U);
    }
    BOOST_CHECK_EQUAL(set.size(), eraseIndex);
    BOOST_CHECK(set.empty() == false);

    BOOST_TEST_MESSAGE("check swap...");
    ContainerType container;
    set.swap(container);

    BOOST_CHECK_EQUAL(set.size(), 0U);
    BOOST_CHECK(set.empty());
    for (int i=0; i<count; ++i)
    {
        BOOST_CHECK_EQUAL(set.count(i), 0U);
    }

    BOOST_CHECK_EQUAL(container.size(), eraseIndex);
    BOOST_CHECK(container.empty() == false);
    for (int i=0; i<eraseIndex; ++i)
    {
        BOOST_CHECK_EQUAL(container.count(i), 1U);
    }
    for (int i=eraseIndex; i<count; ++i)
    {
        BOOST_CHECK_EQUAL(container.count(i), 0U);
    }
}

}

BOOST_AUTO_TEST_SUITE(ConcurrentSetTest)

BOOST_AUTO_TEST_CASE(testSetFunc)
{
    typedef int KeyType;

    BOOST_TEST_MESSAGE("=> check default container type...");
    typedef izenelib::am::ConcurrentSet<KeyType> DefaultSet;
    checkSet<DefaultSet>();

    BOOST_TEST_MESSAGE("=> check std::set as container type...");
    typedef std::set<KeyType> StdContainer;
    typedef izenelib::am::ConcurrentSet<KeyType, StdContainer> StdSet;
    checkSet<StdSet>();
}

BOOST_AUTO_TEST_SUITE_END() 
