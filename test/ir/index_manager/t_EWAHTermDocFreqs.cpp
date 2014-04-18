#include "BitmapTermDocFreqsTestFixture.h"
#include <am/bitmap/ewah.h>
#include <ir/index_manager/utility/EWAHTermDocFreqs.h>

template <typename word_t>
struct FixtureType
{
    typedef izenelib::am::EWAHBoolArray<word_t> bitmap_t;
    typedef izenelib::ir::indexmanager::EWAHTermDocFreqs<word_t> iter_t;

    typedef BitmapTermDocFreqsTestFixture<bitmap_t, iter_t> type;
};

BOOST_AUTO_TEST_SUITE(EWAHTermDocFreqsTest)

BOOST_AUTO_TEST_CASE(docFreq)
{
    FixtureType<uint16_t>::type().testDocFreq();
    FixtureType<uint32_t>::type().testDocFreq();
    FixtureType<uint64_t>::type().testDocFreq();
}

BOOST_AUTO_TEST_CASE(next)
{
    FixtureType<uint16_t>::type().testNext();
    FixtureType<uint32_t>::type().testNext();
    FixtureType<uint64_t>::type().testNext();
}

BOOST_AUTO_TEST_CASE(skipTo)
{
    FixtureType<uint16_t>::type().testSkipTo();
    FixtureType<uint32_t>::type().testSkipTo();
    FixtureType<uint64_t>::type().testSkipTo();
}

BOOST_AUTO_TEST_SUITE_END()
