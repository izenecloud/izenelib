#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

#include <boost/test/unit_test.hpp>

#include <am/succinct/ds-vector/DyWaveletTree.hpp>
#include <am/succinct/ds-vector/Util.hpp>

using namespace std;

BOOST_AUTO_TEST_SUITE( t_ds_vector_suite )

BOOST_AUTO_TEST_CASE(DyWaveletTree_Basic)
{
    izenelib::am::succinct::ds_vector::DyWaveletTree dwt;
    BOOST_CHECK_EQUAL(0, dwt.Size());
    BOOST_CHECK_EQUAL(0, dwt.Rank(0, 0));

    dwt.Clear();
    dwt.Insert(0, 0);
    dwt.Insert(1, 7);
    dwt.Insert(1, 77);
    dwt.Insert(0, 77);

    izenelib::am::succinct::ds_vector::Util::PrintBit(dwt.Lookup(0));
    izenelib::am::succinct::ds_vector::Util::PrintBit(77);

    BOOST_CHECK_EQUAL(77, dwt.Lookup(0));
    BOOST_CHECK_EQUAL(0,  dwt.Lookup(1));
    BOOST_CHECK_EQUAL(77, dwt.Lookup(2));
    BOOST_CHECK_EQUAL(7,  dwt.Lookup(3));

    BOOST_CHECK_EQUAL(0, dwt.Rank(0, 0));
    BOOST_CHECK_EQUAL(1, dwt.Rank(1, 77));
    BOOST_CHECK_EQUAL(1, dwt.Rank(2, 0));
    BOOST_CHECK_EQUAL(2, dwt.Rank(3, 77));
    BOOST_CHECK_EQUAL(1, dwt.Rank(4, 7));
    BOOST_CHECK_EQUAL(2, dwt.Rank(4, 77));
    BOOST_CHECK_EQUAL(0, dwt.Rank(4, 6));

    BOOST_CHECK_EQUAL(4, dwt.Size());
}

BOOST_AUTO_TEST_CASE(DyWaveletTree_Random)
{
    uint64_t N = 10000;
    vector<uint8_t> T;
    izenelib::am::succinct::ds_vector::DyWaveletTree dwt;
    for (uint64_t i = 0; i < N; ++i)
    {
        uint64_t pos = rand() % (i+1);
        uint8_t x = rand() % 0x100;
        dwt.Insert(pos, x);
        T.insert(T.begin() + pos, x);
    }

    BOOST_CHECK_EQUAL(N, dwt.Size());

    for (uint64_t i = 0; i < N; ++i)
    {
        BOOST_CHECK_EQUAL(T[i], dwt.Lookup(i));
    }

    vector<uint64_t> cums(0x100);
    for (uint64_t i = 0; i < N; ++i)
    {
        if ((rand() % 20) == 0)
        {
            for (uint64_t c = 0; c < 0x100; ++c)
            {
                BOOST_CHECK_EQUAL(cums[c], dwt.Rank(i, c));
            }
        }
        cums[T[i]]++;
    }
}


BOOST_AUTO_TEST_SUITE_END()

