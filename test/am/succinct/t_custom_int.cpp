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
    std::vector<int48_t> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.push_back(4);
    v.push_back(5);
    BOOST_CHECK(v[4] == 5);	
    BOOST_CHECK(v[v[1]] == 3);		
    }
}

BOOST_AUTO_TEST_SUITE_END()

