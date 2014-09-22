#include <boost/test/unit_test.hpp>

#include <am/succinct/dbitv/dbitv.hpp>

using namespace std;

#define BITV_SZ 262144LL

BOOST_AUTO_TEST_SUITE( t_dense_succinct_bitv_suite )

BOOST_AUTO_TEST_CASE(rank)
{
    izenelib::am::succinct::dense::DBitV bv(false);
    vector<uint64_t> bits(BITV_SZ / 64);
    for (size_t i = 0; i < BITV_SZ / 64; ++i)
    {
        if (i % 2 == 0)
            bits[i] = 0x5555555555555555;
        else
            bits[i] = 0xAAAAAAAAAAAAAAAA;
    }
    bv.build(bits, BITV_SZ);

    size_t nrank0 = 0;
    size_t nrank1 = 0;
    size_t rank;

    for (int i = 0; i < BITV_SZ; i++)
    {
        if (bv.access(i, rank))
        {
            BOOST_CHECK_EQUAL(nrank1, rank);
            ++nrank1;
        }
        else
        {
            BOOST_CHECK_EQUAL(nrank0, rank);
            ++nrank0;
        }
    }
}

BOOST_AUTO_TEST_CASE(select)
{
    izenelib::am::succinct::dense::DBitV bv(true);
    vector<uint64_t> bits(BITV_SZ / 64);
    for (size_t i = 0; i < BITV_SZ / 64; ++i)
    {
        if (i % 2 == 0)
            bits[i] = 0x5555555555555555;
        else
            bits[i] = 0xAAAAAAAAAAAAAAAA;
    }
    bv.build(bits, BITV_SZ);

    size_t nrank0 = 0;
    size_t nrank1 = 0;

    for (int i = 0; i < BITV_SZ; i++)
    {
        if (bv.access(i))
        {
            BOOST_CHECK_EQUAL(i, bv.select1(nrank1));
            ++nrank1;
        }
        else
        {
            BOOST_CHECK_EQUAL(i, bv.select0(nrank0));
            ++nrank0;
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()
