#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

#include <boost/test/unit_test.hpp>

#include <am/succinct/succinct_vector.hpp>

using namespace std;
BOOST_AUTO_TEST_SUITE( t_succinct_vector_suite )

BOOST_AUTO_TEST_CASE(Insert)
{
    izenelib::am::succinct::vector<double> foo;
    const unsigned limit = 100;
    for(unsigned i = 0; i < limit; ++i) {
        foo.push_back(i);
        BOOST_CHECK (foo.size() == i+1);
        for(unsigned j = 0; j <= sqrt(i); ++j) {
            const double k = rand() % foo.size();
            BOOST_CHECK (k == foo[k]);
            foo[k] = static_cast<double>(k);
        }
        const unsigned little = min(static_cast<unsigned>(sqrt(limit)),i);
        for(unsigned j = 0; j <= little; ++j) {
            foo.pop_back();
        }
        for(unsigned j = i - little; j <= i; ++j) {
            foo.push_back(j);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

