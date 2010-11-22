#include <vector>
#include <sstream> // std::ostringstream
#include <boost/test/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/program_options.hpp>

#include <glog/logging.h>

#include "IndexerTestFixture.h"
#include "t_IndexReader.h"
#include "t_BarrelsInfo.h"
#include "t_TermDocFreqs.h"

using namespace std;
using namespace boost::unit_test;
namespace po = boost::program_options;

bool my_init_unit_test()
{
    IndexerTestConfig configs[] = {
        {0, 1, 1, "default", true},
        {1, 1, 1, "realtime", true},
        {2, 5, 3, "default", true},
        {3, 5, 3, "realtime", true},
        {4, 10, 6, "default", true},
        {5, 10, 6, "realtime", true},
        //{5, 100, 23, "default", true},
        //{6, 100, 23, "realtime", true},
        //{7, 1000, 10, "default", true},
        //{8, 1000, 10, "realtime", true},
    };
    const int configNum = sizeof(configs) / sizeof(IndexerTestConfig);

    // load command line option
    vector<IndexerTestConfig> configVec;
    try
    {
        std::vector<int> runConfigVec;
        po::options_description optDesp("Allowed options");
        optDesp.add_options()
            ("run_config,n", po::value<std::vector<int> >(&runConfigVec)->multitoken(), "specify the range of config parameters to run")
            ;

        po::options_description cmdline_options;
        cmdline_options.add(optDesp);

        po::variables_map vm;
        store(po::command_line_parser(framework::master_test_suite().argc, framework::master_test_suite().argv).options(cmdline_options).run(), vm);
        po::notify(vm);    

        if(vm.count("run_config"))
        {
            cout << "run_config: ";
            for(std::vector<int>::const_iterator it = runConfigVec.begin();
                    it != runConfigVec.end();
                    ++it)
            {
                cout << *it << ", ";
                if(*it >= 0 && *it < configNum)
                    configVec.push_back(configs[*it]);
                else
                {
                    ostringstream oss;
                    oss << "unknown config number " << *it;
                    throw std::runtime_error(oss.str());
                }
            }
            cout << endl;
        }
        else
        {
            cout << "run all configs" << endl;
            configVec.insert(configVec.begin(), configs, configs+configNum);
        }
    }
    catch(std::exception& e)
    {
        cerr << "error: " << e.what() << "\n";
        return false;
    }

    const IndexerTestConfig* const pConfigStart = &configVec[0];
    const IndexerTestConfig* const pConfigEnd = pConfigStart + configVec.size();

    test_suite* tsIndexReader = BOOST_TEST_SUITE("t_IndexReader");
    tsIndexReader->add(BOOST_PARAM_TEST_CASE(&t_IndexReader::index, pConfigStart, pConfigEnd));
    tsIndexReader->add(BOOST_PARAM_TEST_CASE(&t_IndexReader::update, pConfigStart, pConfigEnd));
    tsIndexReader->add(BOOST_PARAM_TEST_CASE(&t_IndexReader::remove, pConfigStart, pConfigEnd));
    framework::master_test_suite().add(tsIndexReader);

    test_suite* tsBarrelsInfo = BOOST_TEST_SUITE("t_BarrelsInfo");
    tsBarrelsInfo->add(BOOST_PARAM_TEST_CASE(&t_BarrelsInfo::index, pConfigStart, pConfigEnd));
    tsBarrelsInfo->add(BOOST_PARAM_TEST_CASE(&t_BarrelsInfo::optimize, pConfigStart, pConfigEnd));
    tsBarrelsInfo->add(BOOST_PARAM_TEST_CASE(&t_BarrelsInfo::createAfterOptimize, pConfigStart, pConfigEnd));
    tsBarrelsInfo->add(BOOST_PARAM_TEST_CASE(&t_BarrelsInfo::pauseResumeMerge, pConfigStart, pConfigEnd));
    framework::master_test_suite().add(tsBarrelsInfo);

    test_suite* tsTermDocFreqs = BOOST_TEST_SUITE("t_TermDocFreqs");
    tsTermDocFreqs->add(BOOST_PARAM_TEST_CASE(&t_TermDocFreqs::index, pConfigStart, pConfigEnd));
    tsTermDocFreqs->add(BOOST_PARAM_TEST_CASE(&t_TermDocFreqs::update, pConfigStart, pConfigEnd));
    tsTermDocFreqs->add(BOOST_PARAM_TEST_CASE(&t_TermDocFreqs::remove, pConfigStart, pConfigEnd));
    framework::master_test_suite().add(tsTermDocFreqs);

    return true;
}

int main( int argc, char* argv[] )
{
    google::InitGoogleLogging(argv[0]);

    return ::boost::unit_test::unit_test_main( &my_init_unit_test, argc, argv );
}
