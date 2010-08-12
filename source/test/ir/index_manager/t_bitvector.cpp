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

using namespace std;
using namespace boost;
using namespace izenelib::ir::indexmanager;

BOOST_AUTO_TEST_SUITE( t_bitvector )

BOOST_AUTO_TEST_CASE(bitvector)
{
    size_t count = 10000;
    BitVector bitvector(count);
    for(size_t i = 0; i < count; ++i)
        bitvector.set(i);
    for(size_t i = 0; i < count; ++i)
        BOOST_CHECK(bitvector.test(i));
    bitvector.clear();
    for(size_t i = 0; i < count; ++i)
        BOOST_CHECK(! bitvector.test(i));
    for(size_t i = 0; i < count; ++i)
        bitvector.set(i);
    BitVector bitvector2(count);
    bitvector2 |= bitvector;
    for(size_t i = 0; i < count; ++i)
        BOOST_CHECK(bitvector2.test(i));
	
}

BOOST_AUTO_TEST_SUITE_END()

