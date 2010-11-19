#include <boost/test/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>

#include <glog/logging.h>

#include "IndexerTestFixture.h"
#include "t_IndexReader.h"
#include "t_BarrelsInfo.h"
#include "t_TermDocFreqs.h"

using namespace boost::unit_test;

bool my_init_unit_test()
{
    IndexerTestConfig configs[] = {
        {0, 1, 1, "default", true},
        {1, 1, 1, "realtime", true},
        {2, 5, 5, "default", true},
        {3, 5, 5, "realtime", true},
    };
    const int configNum = sizeof(configs) / sizeof(IndexerTestConfig);

    test_suite* tsIndexReader = BOOST_TEST_SUITE("t_IndexReader");
    tsIndexReader->add(BOOST_PARAM_TEST_CASE(&t_IndexReader::index, configs, configs+configNum));
    tsIndexReader->add(BOOST_PARAM_TEST_CASE(&t_IndexReader::update, configs, configs+configNum));
    tsIndexReader->add(BOOST_PARAM_TEST_CASE(&t_IndexReader::remove, configs, configs+configNum));
    framework::master_test_suite().add(tsIndexReader);

    test_suite* tsBarrelsInfo = BOOST_TEST_SUITE("t_BarrelsInfo");
    tsBarrelsInfo->add(BOOST_PARAM_TEST_CASE(&t_BarrelsInfo::index, configs, configs+configNum));
    tsBarrelsInfo->add(BOOST_PARAM_TEST_CASE(&t_BarrelsInfo::optimize, configs, configs+configNum));
    tsBarrelsInfo->add(BOOST_PARAM_TEST_CASE(&t_BarrelsInfo::createAfterOptimize, configs, configs+configNum));
    tsBarrelsInfo->add(BOOST_PARAM_TEST_CASE(&t_BarrelsInfo::pauseResumeMerge, configs, configs+configNum));
    framework::master_test_suite().add(tsBarrelsInfo);

    test_suite* tsTermDocFreqs = BOOST_TEST_SUITE("t_TermDocFreqs");
    tsTermDocFreqs->add(BOOST_PARAM_TEST_CASE(&t_TermDocFreqs::index, configs, configs+configNum));
    tsTermDocFreqs->add(BOOST_PARAM_TEST_CASE(&t_TermDocFreqs::update, configs, configs+configNum));
    tsTermDocFreqs->add(BOOST_PARAM_TEST_CASE(&t_TermDocFreqs::remove, configs, configs+configNum));
    framework::master_test_suite().add(tsTermDocFreqs);

    return true;
}

int main( int argc, char* argv[] )
{
    google::InitGoogleLogging(argv[0]);

    return ::boost::unit_test::unit_test_main( &my_init_unit_test, argc, argv );
}
