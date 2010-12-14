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
#include <sstream>
#include <string>

#include <ir/index_manager/utility/BitVector.h>
#include <ir/index_manager/store/FSDirectory.h>


using namespace std;
using namespace boost;
using namespace izenelib::ir::indexmanager;

/**
 * check BitVector functions.
 * @param count bits size to test
 * @param isCheckEachBit whether check each bit in the loop,
 * when @p count is large, it would better to be false to avoid consuming lots of test time.
 */
void checkBitVector(size_t count, bool isCheckEachBit = true)
{
    BOOST_TEST_MESSAGE("checkBitVector, count: " << count);

    BitVector bitvector(count);
    BOOST_CHECK_EQUAL(bitvector.size(), count);
    for(size_t i = 0; i < count && isCheckEachBit; ++i)
        bitvector.set(i);
    for(size_t i = 0; i < count && isCheckEachBit; ++i)
        BOOST_CHECK(bitvector.test(i));
    BOOST_CHECK(! bitvector.test(count));
    BOOST_CHECK(! bitvector.test(2 * count + 1));

    for(size_t i = 0; i < count && isCheckEachBit; ++i)
        bitvector.clear(i);
    for(size_t i = 0; i < count && isCheckEachBit; ++i)
        BOOST_CHECK(! bitvector.test(i));

    bitvector.setAll();
    bitvector.clear();
    for(size_t i = 0; i < count && isCheckEachBit; ++i)
        BOOST_CHECK(! bitvector.test(i));
    BOOST_CHECK(! bitvector.any());

    bitvector.toggle();
    for(size_t i = 0; i < count && isCheckEachBit; ++i)
        BOOST_CHECK(bitvector.test(i));

    bitvector.setAll();
    BitVector bitvector2(count);
    bitvector2 |= bitvector;
    for(size_t i = 0; i < count && isCheckEachBit; ++i)
        BOOST_CHECK(bitvector2.test(i));

    bitvector2 &= bitvector;
    for(size_t i = 0; i < count && isCheckEachBit; ++i)
        BOOST_CHECK(bitvector2.test(i));

    bitvector2 ^= bitvector;
    for(size_t i = 0; i < count && isCheckEachBit; ++i)
        BOOST_CHECK(! bitvector2.test(i));

    // output & intput
    bitvector.setAll();
    const char* dirStr = "bitvector";
    ostringstream ost;
    ost << count << ".bits";
    string fileName = ost.str();
    Directory* pDirectory = new FSDirectory(dirStr, true);
    bitvector.write(pDirectory, fileName.c_str());

    BitVector bitvector3;
    bitvector3.read(pDirectory, fileName.c_str());
    delete pDirectory;
    BOOST_CHECK_EQUAL(bitvector.size(), count);
    for(size_t i = 0; i < count && isCheckEachBit; ++i)
        BOOST_CHECK_EQUAL(bitvector3.test(i), bitvector.test(i));
    BOOST_CHECK(! bitvector3.test(count));
    BOOST_CHECK(! bitvector3.test(2 * count + 1));

    // at middle
    bitvector.clear();
    BOOST_CHECK(! bitvector.hasSmallThan(count));
    BOOST_CHECK(! bitvector.hasBetween(0, count - 1));
    BOOST_CHECK(! bitvector.hasBetween(0, count * 2 + 1));
    BOOST_CHECK(! bitvector.hasBetween(count, count * 2 + 1));
    size_t mid = count / 2;
    bitvector.set(mid);
    BOOST_CHECK(bitvector.any());
    BOOST_CHECK(bitvector.hasSmallThan(count));
    BOOST_CHECK(bitvector.hasSmallThan(mid));
    BOOST_CHECK(bitvector.hasBetween(mid, mid));
    BOOST_CHECK(bitvector.hasBetween(0, count * 2 + 1));
    if(mid > 0)
    {
        BOOST_CHECK(! bitvector.hasSmallThan(mid - 1));
        BOOST_CHECK(bitvector.hasBetween(0, count - 1));
        BOOST_CHECK(! bitvector.hasBetween(0, mid - 1));
        BOOST_CHECK(bitvector.hasBetween(mid, count - 1));
        BOOST_CHECK(bitvector.hasBetween(mid - 1, mid + 1));
        if(mid + 1 <= count - 1)
            BOOST_CHECK(! bitvector.hasBetween(mid + 1, count - 1));
        BOOST_CHECK(! bitvector.hasBetween(count, count * 2 + 1));
    }

    // at boundary
    bitvector.clear(count);
    bitvector.setAll();
    BOOST_CHECK_EQUAL(bitvector.size(), count + 1);
    bitvector.set(count + 4);
    BOOST_CHECK_EQUAL(bitvector.size(), count + 5);
    BOOST_CHECK(bitvector.test(count));
    BOOST_CHECK(! bitvector.test(count + 1));
    BOOST_CHECK(! bitvector.test(count + 2));
    BOOST_CHECK(! bitvector.test(count + 3));
    BOOST_CHECK(bitvector.test(count + 4));
    BOOST_CHECK(! bitvector.test(count + 5));
    BOOST_CHECK(! bitvector.test(count + 6));
    BOOST_CHECK(! bitvector.test(count + 7));
}

BOOST_AUTO_TEST_SUITE(t_bitvector)

BOOST_AUTO_TEST_CASE(bitvector)
{
    for(size_t i=0; i<70; ++i)
        checkBitVector(i);

    checkBitVector(100);
    checkBitVector(1000);
    checkBitVector(100000);
    checkBitVector(10000000, false); // 10M
    checkBitVector(134217728, false); // 128M
    checkBitVector(1073741825, false); // 1G + 1
}

BOOST_AUTO_TEST_SUITE_END()
