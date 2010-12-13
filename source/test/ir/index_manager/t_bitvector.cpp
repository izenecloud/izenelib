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

#include <ir/index_manager/utility/BitVector.h>
#include <ir/index_manager/store/FSDirectory.h>


using namespace std;
using namespace boost;
using namespace izenelib::ir::indexmanager;

BOOST_AUTO_TEST_SUITE( t_bitvector )

BOOST_AUTO_TEST_CASE(bitvector)
{
    size_t count = 10000;

    BitVector bitvector(count);
    BOOST_CHECK_EQUAL(bitvector.size(), count);
    for(size_t i = 0; i < count; ++i)
        bitvector.set(i);
    for(size_t i = 0; i < count; ++i)
        BOOST_CHECK(bitvector.test(i));
    BOOST_CHECK(bitvector.any());

    for(size_t i = 0; i < count; ++i)
        bitvector.clear(i);
    for(size_t i = 0; i < count; ++i)
        BOOST_CHECK(! bitvector.test(i));

    bitvector.setAll();
    bitvector.clear();
    for(size_t i = 0; i < count; ++i)
        BOOST_CHECK(! bitvector.test(i));
    BOOST_CHECK(! bitvector.any());

    bitvector.toggle();
    for(size_t i = 0; i < count; ++i)
        BOOST_CHECK(bitvector.test(i));

    bitvector.clear();
    BOOST_CHECK(! bitvector.hasSmallThan(count));
    BOOST_CHECK(! bitvector.hasBetween(0, count - 1));
    BOOST_CHECK(! bitvector.hasBetween(0, count * 2));
    BOOST_CHECK(! bitvector.hasBetween(count, count * 2));
    int mid = 100;
    bitvector.set(mid);
    BOOST_CHECK(bitvector.hasSmallThan(count));
    BOOST_CHECK(bitvector.hasSmallThan(mid));
    BOOST_CHECK(! bitvector.hasSmallThan(mid - 1));
    BOOST_CHECK(bitvector.hasBetween(0, count - 1));
    BOOST_CHECK(! bitvector.hasBetween(0, mid - 1));
    BOOST_CHECK(bitvector.hasBetween(mid, mid));
    BOOST_CHECK(bitvector.hasBetween(mid, count - 1));
    BOOST_CHECK(! bitvector.hasBetween(mid + 1, count - 1));
    BOOST_CHECK(bitvector.hasBetween(0, count * 2));
    BOOST_CHECK(! bitvector.hasBetween(count, count * 2));

    bitvector.setAll();
    BitVector bitvector2(count);
    bitvector2 |= bitvector;
    for(size_t i = 0; i < count; ++i)
        BOOST_CHECK(bitvector2.test(i));

    bitvector2 &= bitvector;
    for(size_t i = 0; i < count; ++i)
        BOOST_CHECK(bitvector2.test(i));

    bitvector2 ^= bitvector;
    for(size_t i = 0; i < count; ++i)
        BOOST_CHECK(! bitvector2.test(i));

    bitvector.setAll();
    const char* dirStr = "bitvector";
    const char* fileStr = "0.bits";
    Directory* pDirectory = new FSDirectory(dirStr, true);
    bitvector.write(pDirectory, fileStr);

    BitVector bitvector3;
    bitvector3.read(pDirectory, fileStr);
    for(size_t i = 0; i < count; ++i)
        BOOST_CHECK_EQUAL(bitvector3.test(i), bitvector.test(i));
    bitvector3.write(pDirectory, "3.bits");
    delete pDirectory;

    bitvector.clear(count);
    bitvector.setAll();
    BOOST_CHECK(bitvector.test(count));
    BOOST_CHECK(! bitvector.test(count + 1));
    bitvector.set(count + 4);
    BOOST_CHECK(bitvector.test(count + 4));
    BOOST_CHECK(! bitvector.test(count + 2));
    BOOST_CHECK(! bitvector.test(count + 3));
}

BOOST_AUTO_TEST_CASE(bitvector_empty)
{
    BitVector bitvector;
    bitvector.setAll();
    bitvector.clear();

    BOOST_CHECK_EQUAL(bitvector.size(), 0);
    BOOST_CHECK(! bitvector.test(0));
    BOOST_CHECK(! bitvector.test(1));
    BOOST_CHECK(! bitvector.test(10));
    BOOST_CHECK(! bitvector.any());
    BOOST_CHECK(! bitvector.hasSmallThan(0));
    BOOST_CHECK(! bitvector.hasBetween(0, 10));

    const char* dirStr = "bitvector";
    const char* fileStr = "empty_0.bits";
    Directory* pDirectory = new FSDirectory(dirStr, true);
    bitvector.write(pDirectory, fileStr);

    BitVector bitvector1;
    bitvector1.read(pDirectory, fileStr);
    bitvector1.write(pDirectory, "empty_1.bits");
    BOOST_CHECK_EQUAL(bitvector1.size(), 0);
    BOOST_CHECK(! bitvector1.test(0));
    BOOST_CHECK(! bitvector1.test(1));
    BOOST_CHECK(! bitvector1.test(10));
    BOOST_CHECK(! bitvector1.any());
    BOOST_CHECK(! bitvector1.hasSmallThan(0));
    BOOST_CHECK(! bitvector1.hasBetween(0, 10));

    delete pDirectory;
}

BOOST_AUTO_TEST_SUITE_END()
