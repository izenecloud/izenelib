#include <boost/test/unit_test.hpp>

#include <util/math.h>
#include <cmath>
#include <boost/random.hpp>


using namespace izenelib::util;


BOOST_AUTO_TEST_SUITE(t_math_util)

BOOST_AUTO_TEST_CASE(math_test)
{
    boost::mt19937 engine ;
    boost::uniform_int<> distribution(1,1000000);
    boost::variate_generator<boost::mt19937, boost::uniform_int<> > random(engine,distribution);
	
    int size = 10;
    for(int i = 0; i < size; ++i)
    {
        int r = random();
        std::cout<<log10(r)<<" "<<fastlog10(r)<<" "<<fasterlog10(r)<<std::endl;
    }
}


BOOST_AUTO_TEST_SUITE_END()

