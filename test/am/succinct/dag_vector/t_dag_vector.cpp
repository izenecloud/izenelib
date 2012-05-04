#include <boost/test/unit_test.hpp>
#include <stdlib.h>
#include <iostream>
#include <am/succinct/dag_vector/dag_vector.hpp>

using namespace std;

using namespace boost::unit_test;


BOOST_AUTO_TEST_SUITE(dag_vector_test)

BOOST_AUTO_TEST_CASE(dag_vector)
{
    int n = 10000;
    int vmax = 100;

    cout << "n=" << n << " vmax=" << vmax << endl;
    izenelib::am::succinct::dag_vector dagv;
    vector<uint64_t> vals;
    vector<uint64_t> sums;
    uint64_t sum = 0;
    for (int i = 0; i < n; ++i)
    {
        uint64_t val = rand() % vmax;
        dagv.push_back(val);
        vals.push_back(val);
        sums.push_back(sum);
        sum += val;
    }

    cout<<"get_alloc_byte_num "<<dagv.get_alloc_byte_num()<<endl;
    BOOST_CHECK(dagv.size() == (size_t)n);

    for (uint64_t i = 0; i < vals.size(); ++i)
    {
        if (dagv[i] != vals[i])
        {
            cout << "Error [] i=" << i << " dagv[i]=" << dagv[i] << " vals[i]=" << vals[i] << endl;
        }
        if (dagv.prefix_sum(i) != sums[i])
        {
            cout << "Error prefix_sum i=" << i << " dagv.prefix_sum(i)=" << dagv.prefix_sum(i) << " prefix_sums[i]=" << sums[i] << endl;
        }
    }

    size_t pos = 0;
    for (izenelib::am::succinct::dag_vector::const_iterator it = dagv.begin(); it != dagv.end(); ++it, ++pos)
    {
        if (*it != vals[pos])
        {
            cout << "Error Iter i=" << pos << " dagv[i]=" << *it << " vals[i]=" << vals[pos] << endl;
        }
    }

    cout << "test passed." << endl;

}

BOOST_AUTO_TEST_SUITE_END() // dag_vector_test

