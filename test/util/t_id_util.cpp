#include <boost/test/unit_test.hpp>

#include <util/id_util.h>


using namespace izenelib::util;


BOOST_AUTO_TEST_SUITE(t_id_util)

BOOST_AUTO_TEST_CASE(normal_test)
{
    uint32_t id1 = 678;
    uint32_t id2 = 2341;
    uint64_t id = IdUtil::Get64(id1, id2);
    std::pair<uint32_t, uint32_t> r = IdUtil::Get32(id);
    BOOST_CHECK( r.first==id1 );
    BOOST_CHECK( r.second==id2 );
}


BOOST_AUTO_TEST_SUITE_END()
