#include "EWAHTermDocFreqsTestFixture.h"

BOOST_AUTO_TEST_SUITE(EWAHTermDocFreqsTest)

BOOST_AUTO_TEST_CASE(testDocFreq)
{
    EWAHTermDocFreqsTestFixture<uint16_t>().testDocFreq();
    EWAHTermDocFreqsTestFixture<uint32_t>().testDocFreq();
    EWAHTermDocFreqsTestFixture<uint64_t>().testDocFreq();
}

BOOST_AUTO_TEST_CASE(testNext)
{
    EWAHTermDocFreqsTestFixture<uint16_t>().testNext();
    EWAHTermDocFreqsTestFixture<uint32_t>().testNext();
    EWAHTermDocFreqsTestFixture<uint64_t>().testNext();
}

BOOST_AUTO_TEST_CASE(testSkipTo)
{
    EWAHTermDocFreqsTestFixture<uint16_t>().testSkipTo();
    EWAHTermDocFreqsTestFixture<uint32_t>().testSkipTo();
    EWAHTermDocFreqsTestFixture<uint64_t>().testSkipTo();
}

BOOST_AUTO_TEST_SUITE_END()
