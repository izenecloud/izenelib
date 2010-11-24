#include <vector>
#include <sstream> // std::ostringstream
#include <cstdlib> //getenv
#include <boost/test/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/program_options.hpp>
#include <boost/foreach.hpp>
#include <boost/test/output/xml_log_formatter.hpp>

#include <glog/logging.h>

#include <TestHelper.h> //MyXmlLogFormatter
#include "IndexerTestFixture.h"
#include "t_IndexReader.h"
#include "t_BarrelsInfo.h"
#include "t_TermDocFreqs.h"

using namespace std;
using namespace boost::unit_test;
namespace po = boost::program_options;

namespace {
/** the parameters for each test case */
const IndexerTestConfig INDEXER_TEST_CONFIGS[] = {
    {0, 1, 1, "default", true},
    {1, 1, 1, "realtime", true},
    {2, 5, 3, "default", true},
    {3, 5, 3, "realtime", true},
    {4, 10, 6, "default", true},
    {5, 10, 6, "realtime", true},
    {6, 100, 23, "default", true},
    {7, 100, 23, "realtime", true},
    {8, 1000, 10, "default", true},
    {9, 1000, 10, "realtime", true},
    {10, 1, 1, "default:block", true},
    {11, 1, 1, "default:chunk", true},
};

/** the parameter number */
const int INDEXER_TEST_CONFIG_NUM = sizeof(INDEXER_TEST_CONFIGS) / sizeof(IndexerTestConfig);

/** if the command options of "--run_config_list" or "--run_config_range" are not specified, run configs of range [0, DEFAULT_CONFIG_RANGE] */
const int DEFAULT_CONFIG_RANGE = 5;

/** the command option to specify a list of config numbers, such as "--run_config_list 0 1 2 3" */
const char* OPTION_CONFIG_LIST = "run_config_list";

/** the command option to specify a range of config numbers, such as "--run_config_range 4 7" */
const char* OPTION_CONFIG_RANGE = "run_config_range";

/** the environment variable name for boost test log */
const char* ENV_NAME_LOG_FILE = "BOOST_TEST_LOG_FILE";
}

static std::ofstream* gOutStream = 0;

/**
 * Load command line option "run_config_list" or "run_config_range" to decide which configs to run.
 * @param configVec the loaded configs to run
 * @return true for load success, false for fail
 */
bool loadConfigOption(vector<IndexerTestConfig>& configVec)
{
    cout << "hello world" << endl;
    try
    {
        vector<int> runConfigListVec;
        vector<int> runConfigRangeVec;
        po::options_description desp("Allowed options");
        desp.add_options()
            ("build_info,i", "this option is not used, it is just in case of boost test didn't filter out this option")
            (OPTION_CONFIG_LIST, po::value<std::vector<int> >(&runConfigListVec)->multitoken(), "specify the list of config parameters to run")
            (OPTION_CONFIG_RANGE, po::value<std::vector<int> >(&runConfigRangeVec)->multitoken(), "specify the range of config parameters to run")
            ;

        po::options_description cmdline_options;
        cmdline_options.add(desp);

        po::variables_map vm;
        store(po::command_line_parser(framework::master_test_suite().argc, framework::master_test_suite().argv).options(cmdline_options).run(), vm);
        po::notify(vm);    

        if(vm.count(OPTION_CONFIG_LIST))
        {
            cout << OPTION_CONFIG_LIST << ": ";
            BOOST_FOREACH(int i, runConfigListVec)
            {
                cout << i << ", ";
                if(i >= 0 && i < INDEXER_TEST_CONFIG_NUM)
                    configVec.push_back(INDEXER_TEST_CONFIGS[i]);
                else
                {
                    cout << endl;
                    ostringstream oss;
                    oss << "unknown config number " << i;
                    throw std::runtime_error(oss.str());
                }
            }
            cout << endl;
        }
        else if(vm.count(OPTION_CONFIG_RANGE))
        {
            cout << OPTION_CONFIG_RANGE << ": ";
            BOOST_FOREACH(int i, runConfigRangeVec)
                cout << i << ", ";
            cout << endl;

            if(runConfigRangeVec.size() == 2
                    && runConfigRangeVec[0] >= 0
                    && runConfigRangeVec[0] <= runConfigRangeVec[1]
                    && runConfigRangeVec[1] < INDEXER_TEST_CONFIG_NUM)
                configVec.insert(configVec.begin(), INDEXER_TEST_CONFIGS + runConfigRangeVec[0],
                                                    INDEXER_TEST_CONFIGS + runConfigRangeVec[1] + 1);
            else
            {
                ostringstream oss;
                oss << "the config range is invalid";
                throw std::runtime_error(oss.str());
            }
        }
        else
        {
            cout << "run default configs range: [0, " << DEFAULT_CONFIG_RANGE << "]" << endl;
            configVec.insert(configVec.begin(), INDEXER_TEST_CONFIGS,
                                                INDEXER_TEST_CONFIGS + DEFAULT_CONFIG_RANGE + 1);
        }
    }
    catch(std::exception& e)
    {
        cerr << "error: " << e.what() << endl;
        return false;
    }

    return true;
}

/**
 * Boost test module initialize function, it regsiters the test cases to run.
 * @return true for success, false for failure
 */
bool my_init_unit_test()
{
    if(const char* const gEnvValue = std::getenv(ENV_NAME_LOG_FILE))
    {
        cout << ENV_NAME_LOG_FILE << ": " << gEnvValue << endl;
        gOutStream = new std::ofstream(gEnvValue);
        boost::unit_test::unit_test_log.set_stream(*gOutStream);
        boost::unit_test::unit_test_log.set_formatter(new MyXmlLogFormatter());
    }

    vector<IndexerTestConfig> configVec;
    if(! loadConfigOption(configVec))
        return false;

    framework::master_test_suite().p_name.value = "index_manager";

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

    int status = ::boost::unit_test::unit_test_main( &my_init_unit_test, argc, argv );

    // delete global stream to flush
    delete gOutStream;

    return status;
}
