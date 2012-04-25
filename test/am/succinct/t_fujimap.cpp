#include <iostream>
#include <string>
#include <fstream>
#include <cassert>
#include <map>
#include <cstring>
#include <cstdlib>

#include <boost/test/unit_test.hpp>

#include <am/succinct/fujimap/fujimap.hpp>

#include <time.h>
#include <sys/time.h>
#include <stdio.h>

using namespace std;

double gettimeofday_sec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + (double)tv.tv_usec*1e-6;
}

BOOST_AUTO_TEST_SUITE( t_fujimap )

BOOST_AUTO_TEST_CASE(Insert)
{
    static uint64_t N = 1000;

    izenelib::am::succinct::fujimap::Fujimap fm;
    fm.initFP(0);
    fm.initTmpN(0);

    char buf[100];
    double t1 = gettimeofday_sec();
    for (uint64_t i = 0; i < N; ++i)
    {
        snprintf(buf, 100, "%llu", (unsigned long long int)i);
        fm.setInteger(buf, strlen(buf), i, false);
    }
    fm.build();
    double t2 = gettimeofday_sec();
    fprintf(stderr, "fm  set   : %f (%f)\n", t2 - t1, (t2 - t1) / N);

    int dummy = 0;
    for (uint64_t i = 0; i < N; ++i)
    {
        snprintf(buf, 100, "%llu", (unsigned long long int)i);
        dummy += fm.getInteger(buf, strlen(buf));
    }
    double t3 = gettimeofday_sec();
    fprintf(stderr, "fm  lookup: %f (%f)\n", t3 - t2, (t3 - t2) / N);
    cerr <<"fm    size: " << fm.getWorkingSize() << endl;

}


BOOST_AUTO_TEST_SUITE_END()

