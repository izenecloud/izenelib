#include <iostream>
#include <vector>

#include <boost/test/unit_test.hpp>

#include <am/succinct/fm-index/custom_int.hpp>

using namespace std;
using izenelib::am::succinct::fm_index::int48_t;
BOOST_AUTO_TEST_SUITE( t_custom_int_suite )

BOOST_AUTO_TEST_CASE(dummy)
{
    {
    int48_t i(10);
    BOOST_CHECK(++i == 11);
    BOOST_CHECK(i++ == 11);	
    BOOST_CHECK((i + 10) == 22);	
    BOOST_CHECK((i - 12) == 0);
    }
    {
    int48_t i(10);
    BOOST_CHECK(--i == 9);
    BOOST_CHECK(i-- == 9);
    BOOST_CHECK((i - 8) == 0);
    }
    {
    int48_t i(5);
    std::vector<int48_t> v;
    v.reserve(i);
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.push_back(4);
    v.push_back(5);
    BOOST_CHECK(v[4] == 5);	
    BOOST_CHECK(v[v[1]] == 3);
    BOOST_CHECK(v[--i] == 5);	    
    BOOST_CHECK(v[--i] == 4);
    int48_t* p = (int48_t *)&v[0];
    BOOST_CHECK(*(p + 1) == 2);
    BOOST_CHECK(*(p + 2) == 3);
    BOOST_CHECK(*(p + 3) == 4);
    BOOST_CHECK(*(p + 4) == 5);
    ++v[4];
    BOOST_CHECK(v[4] == 6);	
    }
    {
    int48_t i(0x7FFFFFFFFFFF);
    int48_t j = ~i;
    int48_t jj = ~j;
    BOOST_CHECK(jj == i);	
    }
    BOOST_CHECK(std::numeric_limits<int48_t>::min() > std::numeric_limits<int64_t>::min() );
    BOOST_CHECK(std::numeric_limits<int48_t>::max() < std::numeric_limits<int64_t>::max() );
}

BOOST_AUTO_TEST_SUITE_END()

