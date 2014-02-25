#include "BitmapTermDocFreqsTestFixture.h"
#include <ir/index_manager/utility/Bitset.h>
#include <ir/index_manager/utility/BitsetTermDocFreqs.h>

typedef izenelib::ir::indexmanager::Bitset bitmap_t;
typedef izenelib::ir::indexmanager::BitsetTermDocFreqs iter_t;
typedef BitmapTermDocFreqsTestFixture<bitmap_t, iter_t> FixtureType;

BOOST_FIXTURE_TEST_SUITE(BitsetTermDocFreqsTest, FixtureType)

BOOST_AUTO_TEST_CASE(docFreq)
{
    testDocFreq();
}

BOOST_AUTO_TEST_CASE(next)
{
    testNext();
}

BOOST_AUTO_TEST_CASE(skipTo)
{
    testSkipTo();
}

BOOST_AUTO_TEST_SUITE_END()
