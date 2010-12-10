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
    BOOST_CHECK(! bitvector.hasBetween(0, count - 1)); // need to check range, need to check bit?
    int mid = 100;
    bitvector.set(mid);
    BOOST_CHECK(bitvector.hasSmallThan(count));
    BOOST_CHECK(bitvector.hasSmallThan(mid));
    BOOST_CHECK(! bitvector.hasSmallThan(mid - 1)); //bug expected, need to check bit?
    BOOST_CHECK(bitvector.hasBetween(0, count - 1));
    BOOST_CHECK_EQUAL(bitvector.getMaxSet(), mid);

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
    for(size_t i = 0; i < count; ++i)
        BOOST_CHECK(bitvector.test(i));
    BitVector bitvector3;
    bitvector3.read(pDirectory, fileStr);
    for(size_t i = 0; i < count; ++i)
        BOOST_CHECK_MESSAGE(bitvector3.test(i), "check bitvector3.test(" << i << ") failed");
    bitvector3.write(pDirectory, "3.bits");
    delete pDirectory;
}

BOOST_AUTO_TEST_SUITE_END()

