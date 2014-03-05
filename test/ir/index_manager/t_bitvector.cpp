#include <boost/test/unit_test.hpp>

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/ref.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/random.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <cstdlib>

#include <ir/index_manager/utility/Bitset.h>
#include <ir/index_manager/store/FSDirectory.h>
#include <util/ClockTimer.h>

using namespace std;
using namespace boost;
using namespace izenelib::ir::indexmanager;

/**
 * check Bitset functions.
 * @param count bits size to test
 * @param isCheckEachBit whether check each bit in the loop,
 * when @p count is large, it would better to be false to avoid consuming lots of test time.
 */
void checkBitset(size_t count, bool isCheckEachBit = true)
{
    BOOST_TEST_MESSAGE("checkBitset, count: " << count);

    Bitset bitset(count);
    BOOST_CHECK_EQUAL(bitset.size(), count);
    for(size_t i = 0; i < count && isCheckEachBit; ++i)
        bitset.set(i);
    for(size_t i = 0; i < count && isCheckEachBit; ++i)
        BOOST_CHECK(bitset.test(i));
    BOOST_CHECK(!bitset.test(count));
    BOOST_CHECK(!bitset.test(2 * count + 1));

    for(size_t i = 0; i < count && isCheckEachBit; ++i)
        bitset.reset(i);
    for(size_t i = 0; i < count && isCheckEachBit; ++i)
        BOOST_CHECK(!bitset.test(i));

    bitset.set();
    bitset.flip();
    for(size_t i = 0; i < count && isCheckEachBit; ++i)
        BOOST_CHECK(!bitset.test(i));
    BOOST_CHECK(!bitset.any());

    bitset.flip();
    for(size_t i = 0; i < count && isCheckEachBit; ++i)
        BOOST_CHECK(bitset.test(i));

    bitset.set();
    Bitset bitvector2(count);
    bitvector2 |= bitset;
    for(size_t i = 0; i < count && isCheckEachBit; ++i)
        BOOST_CHECK(bitvector2.test(i));

    bitvector2 &= bitset;
    for(size_t i = 0; i < count && isCheckEachBit; ++i)
        BOOST_CHECK(bitvector2.test(i));

    bitvector2 ^= bitset;
    for(size_t i = 0; i < count && isCheckEachBit; ++i)
        BOOST_CHECK(!bitvector2.test(i));

    // output & intput
    bitset.set();
    const char* dirStr = "bitset";
    ostringstream ost;
    ost << count << ".bits";
    string fileName = ost.str();
    Directory* pDirectory = new FSDirectory(dirStr, true);
    bitset.write(pDirectory, fileName.c_str());

    Bitset bitvector3;
    bitvector3.read(pDirectory, fileName.c_str());
    delete pDirectory;
    BOOST_CHECK_EQUAL(bitset.size(), count);
    for(size_t i = 0; i < count && isCheckEachBit; ++i)
        BOOST_CHECK_EQUAL(bitvector3.test(i), bitset.test(i));
    BOOST_CHECK(!bitvector3.test(count));
    BOOST_CHECK(!bitvector3.test(2 * count + 1));

    // at middle
    bitset.reset();
    BOOST_CHECK(!bitset.any(0, count));
    BOOST_CHECK(!bitset.any(0, count * 2 + 2));
    BOOST_CHECK(!bitset.any(count, count * 2 + 2));
    size_t mid = count / 2;
    bitset.set(mid);
    BOOST_CHECK(bitset.any());
    BOOST_CHECK(bitset.any(0, count + 1));
    BOOST_CHECK(bitset.any(0, mid + 1));
    BOOST_CHECK(bitset.any(mid, mid + 1));
    BOOST_CHECK(bitset.any(0, count * 2 + 2));
    std::swap(bitset, bitvector2);
    std::swap(bitset, bitvector2);
    if(mid > 0)
    {
        BOOST_CHECK(!bitset.any(0, mid - 1));
        BOOST_CHECK(bitset.any(0, count));
        BOOST_CHECK(!bitset.any(0, mid));
        BOOST_CHECK(bitset.any(mid, count));
        BOOST_CHECK(bitset.any(mid - 1, mid + 2));
        if(mid + 1 <= count - 1)
            BOOST_CHECK(!bitset.any(mid + 1, count));
        BOOST_CHECK(!bitset.any(count, count * 2 + 2));
    }

    // at boundary
    bitset.reset(count);
    bitset.set();
    BOOST_CHECK_EQUAL(bitset.size(), count + 1);
    bitset.set(count + 4);
    BOOST_CHECK_EQUAL(bitset.size(), count + 5);
    BOOST_CHECK(bitset.test(count));
    BOOST_CHECK(!bitset.test(count + 1));
    BOOST_CHECK(!bitset.test(count + 2));
    BOOST_CHECK(!bitset.test(count + 3));
    BOOST_CHECK(bitset.test(count + 4));
    BOOST_CHECK(!bitset.test(count + 5));
    BOOST_CHECK(!bitset.test(count + 6));
    BOOST_CHECK(!bitset.test(count + 7));
}

void testIterate(std::size_t maxBitNum, std::size_t setBitNum)
{
    Bitset bitset(maxBitNum);
    std::set<size_t> goldSet;

    // init
    for (std::size_t i = 0; i < setBitNum; ++i)
    {
        int k = std::rand() % maxBitNum;
        bitset.set(k);
        goldSet.insert(k);
    }

    // forward iterate
    size_t pos = bitset.find_first();
    for (std::set<size_t>::const_iterator it = goldSet.begin();
         it != goldSet.end(); ++it)
    {
        BOOST_CHECK_EQUAL(pos, *it);
        pos = bitset.find_next(pos);
    }
    BOOST_CHECK_EQUAL(pos, -1);

    // backward iterate
    pos = bitset.find_last();
    for (std::set<size_t>::const_reverse_iterator it = goldSet.rbegin();
         it != goldSet.rend(); ++it)
    {
        BOOST_CHECK_EQUAL(pos, *it);
        pos = bitset.find_prev(pos);
    }
    BOOST_CHECK_EQUAL(pos, -1);
}

void setBitset(Bitset& bitset, std::size_t setBitNum)
{
    std::size_t bitNum = bitset.size();
    for (std::size_t i = 0; i < setBitNum; ++i)
    {
        int k = std::rand() % bitNum;
        bitset.set(k);
    }
}

void testLogicalNotAnd(std::size_t bitNum)
{
    Bitset bitVector1(bitNum);
    Bitset bitVector2(bitNum);

    const std::size_t setBitNum = bitNum / 2;
    setBitset(bitVector1, setBitNum);
    setBitset(bitVector2, setBitNum);

    Bitset gold(bitNum);
    for (std::size_t i = 0; i < bitNum; ++i)
    {
        if (bitVector1.test(i) && bitVector2.test(i) == false)
        {
            gold.set(i);
        }
    }

    bitVector1 -= bitVector2;

    BOOST_CHECK_EQUAL(bitVector1, gold);
}

void runLogicalNotAnd(std::size_t bitNum, std::size_t runNum)
{
    const std::size_t setBitNum = bitNum / 2;

    Bitset bitVector1(bitNum);
    Bitset bitVector2(bitNum);

    setBitset(bitVector1, setBitNum);
    setBitset(bitVector2, setBitNum);

    izenelib::util::ClockTimer timer;

    for (std::size_t i = 0; i < runNum; ++i)
    {
        bitVector1 -= bitVector2;
    }

    BOOST_TEST_MESSAGE("it costs " << timer.elapsed() << " seconds in running "
                       "Bitset::operator-=() of " << runNum << " times "
                       "on " << bitNum << " bits");
}

void runToggle(std::size_t bitNum, std::size_t runNum)
{
    Bitset bitset(bitNum);
    const std::size_t setBitNum = bitNum / 2;
    setBitset(bitset, setBitNum);

    izenelib::util::ClockTimer timer;

    for (std::size_t i = 0; i < runNum; ++i)
    {
        bitset.flip();
    }

    BOOST_TEST_MESSAGE("it costs " << timer.elapsed() << " seconds in running "
                       "Bitset::flip() of " << runNum << " times "
                       "on " << bitNum << " bits");
}

BOOST_AUTO_TEST_SUITE(t_bitvector)

BOOST_AUTO_TEST_CASE(bitset)
{
    for(size_t i=0; i<70; ++i)
        checkBitset(i);

    checkBitset(100, false);
    checkBitset(1000, false);
    checkBitset(100000, false);
    checkBitset(10000000, false); // 10M
    checkBitset(134217728, false); // 128M
    checkBitset(1073741825, false); // 1G + 1
}

BOOST_AUTO_TEST_CASE(iterate)
{
    testIterate(10, 0);
    testIterate(10, 10);
    testIterate(100, 10);
    testIterate(1000, 10);
    testIterate(1000, 500);
    testIterate(10000, 1000);
}

BOOST_AUTO_TEST_CASE(logicalNotAnd)
{
    const std::size_t maxBitNum = 100;

    for (std::size_t i = 0; i <= maxBitNum; ++i)
    {
        testLogicalNotAnd(i);
    }
}

BOOST_AUTO_TEST_CASE(benchLogicalNotAnd)
{
    const std::size_t bitNum = 1e7;
    const std::size_t runNum = 1e4;

    runLogicalNotAnd(bitNum, runNum);
}

BOOST_AUTO_TEST_CASE(benchToggle)
{
    const std::size_t bitNum = 1e7;
    const std::size_t runNum = 1e4;

    runToggle(bitNum, runNum);
}

BOOST_AUTO_TEST_SUITE_END()
