#include <boost/test/unit_test.hpp>
#include <iostream>
#include <am/succinct/dag_vector/comp_vector.hpp>
#include <string>
#include <cstdio>

using namespace std;

BOOST_AUTO_TEST_SUITE(dag_vector_test)

BOOST_AUTO_TEST_CASE(comp_vector)
{
    int n = 10000;
    cout << "n=" << n << endl;

    izenelib::am::succinct::comp_vector<string> cv;
    vector<string> origs;
    char buf[256];
    for (int i = 0; i < n; ++i)
    {
        snprintf(buf, 256, "%d", rand() % 1000);
        cv.push_back(string(buf));
        origs.push_back(buf);
    }

    for (size_t i = 0; i < origs.size(); ++i)
    {
        if (origs[i] != cv[i])
        {
            cout << "Error () i=" << i << " " << origs[i] << " " << cv[i] << endl;
        }
    }

    uint64_t pos = 0;
    for (izenelib::am::succinct::comp_vector<string>::const_iterator it = cv.begin(); it != cv.end(); ++it, ++pos)
    {
        if (origs[pos] != *it)
        {
            cout << "Error iter i=" << pos << " " << origs[pos] << " " << *it << endl;
        }
    }

    cout << "test passed." << endl;
}
BOOST_AUTO_TEST_SUITE_END() // dag_vector_test
