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

uint64_t gettimeofday_usec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

BOOST_AUTO_TEST_SUITE( t_fujimap )

BOOST_AUTO_TEST_CASE(Insert)
{
    static uint64_t N = 100;

    izenelib::am::succinct::fujimap::Fujimap<uint128_t, uint32_t> fm("tmp.kf");
    fm.initFP(32);
    fm.initTmpN(10000000);

    uint64_t t1 = gettimeofday_usec();
    for (uint64_t i = 0; i < N; ++i)
    {
        if (i % 1000000 == 999999)
            fprintf(stderr, "fm inserted: %lu\n", i + 1);
        fm.setInteger(i, i, true);
    }
    uint64_t t2 = gettimeofday_usec();
    fprintf(stderr, "fm  set   : %lu (%f)\n", t2 - t1, double(t2 - t1) / N);

    uint64_t fp = 0;
    for (uint64_t i = 0; i < N; ++i)
    {
        if (i % 1000000 == 999999)
            fprintf(stderr, "fm queried: %lu\n", i + 1);
        uint64_t tmp = fm.getInteger(i);
        if (i != tmp) ++fp;
    }
    uint64_t t3 = gettimeofday_usec();
    fprintf(stderr, "fm  lookup: %lu (%f)\n", t3 - t2, double(t3 - t2) / N);
    cerr <<"fm    size: " << fm.getWorkingSize() << endl;
    fprintf(stderr, "false positive: %lu\n", fp);
    fm.save("tmp.fm");
    uint64_t t4 = gettimeofday_usec();
    fprintf(stderr, "fm  serialize: %lu\n", t4 - t3);
    fm.clear();
    fm.load("tmp.fm");
    uint64_t t5 = gettimeofday_usec();
    fprintf(stderr, "fm  deserialize: %lu\n", t5 - t4);
}


BOOST_AUTO_TEST_SUITE_END()
