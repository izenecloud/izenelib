/**
* @file       t_integration_Indexer.cpp
* @author     Jun
* @version    SF1 v5.0
* @brief Integration test suites for index manager.
*
*/

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
/** the parameters for test cases on different data size */
const IndexerTestConfig INDEXER_TEST_CONFIGS[] = {
    // 1
    {0,  1, 1, DOCLEVEL, "default", true},
    {1,  1, 1, DOCLEVEL, "default", true, 8, 3},
    {2,  1, 1, DOCLEVEL, "default", true, 128, 3},
    {3,  1, 1, DOCLEVEL, "realtime", true},
    {4,  1, 1, DOCLEVEL, "realtime", true, 8, 3},
    {5,  1, 1, DOCLEVEL, "realtime", true, 128, 3},
    {6,  1, 1, DOCLEVEL, "default:chunk", true},
    {7,  1, 1, DOCLEVEL, "default:chunk", true, 128, 3},
    {8,  1, 1, DOCLEVEL, "default:block", true},

    {9,  1, 1, WORDLEVEL, "default", true},
    {10, 1, 1, WORDLEVEL, "default", true, 8, 3},
    {11, 1, 1, WORDLEVEL, "default", true, 128, 3},
    {12, 1, 1, WORDLEVEL, "realtime", true},
    {13, 1, 1, WORDLEVEL, "realtime", true, 8, 3},
    {14, 1, 1, WORDLEVEL, "realtime", true, 128, 3},
    {15, 1, 1, WORDLEVEL, "default:chunk", true},
    {16, 1, 1, WORDLEVEL, "default:chunk", true, 128, 3},
    {17, 1, 1, WORDLEVEL, "default:block", true},

    // 5
    {18, 5, 3, DOCLEVEL, "default", true},
    {19, 5, 3, DOCLEVEL, "default", true, 8, 3},
    {20, 5, 3, DOCLEVEL, "default", true, 128, 3},
    {21, 5, 3, DOCLEVEL, "realtime", true},
    {22, 5, 3, DOCLEVEL, "realtime", true, 8, 3},
    {23, 5, 3, DOCLEVEL, "realtime", true, 128, 3},
    {24, 5, 3, DOCLEVEL, "default:chunk", true},
    {25, 5, 3, DOCLEVEL, "default:chunk", true, 128, 3},
    {26, 5, 3, DOCLEVEL, "default:block", true},

    {27, 5, 3, WORDLEVEL, "default", true},
    {28, 5, 3, WORDLEVEL, "default", true, 8, 3},
    {29, 5, 3, WORDLEVEL, "default", true, 128, 3},
    {30, 5, 3, WORDLEVEL, "realtime", true},
    {31, 5, 3, WORDLEVEL, "realtime", true, 8, 3},
    {32, 5, 3, WORDLEVEL, "realtime", true, 128, 3},
    {33, 5, 3, WORDLEVEL, "default:chunk", true},
    {34, 5, 3, WORDLEVEL, "default:chunk", true, 128, 3},
    {35, 5, 3, WORDLEVEL, "default:block", true},

    // 10
    {36, 10, 10, DOCLEVEL, "default", true},
    {37, 10, 10, DOCLEVEL, "default", true, 8, 3},
    {38, 10, 10, DOCLEVEL, "default", true, 128, 3},
    {39, 10, 10, DOCLEVEL, "realtime", true},
    {40, 10, 10, DOCLEVEL, "realtime", true, 8, 3},
    {41, 10, 10, DOCLEVEL, "realtime", true, 128, 3},
    {42, 10, 10, DOCLEVEL, "default:chunk", true},
    {43, 10, 10, DOCLEVEL, "default:chunk", true, 128, 3},
    {44, 10, 10, DOCLEVEL, "default:block", true},

    {45, 10, 10, WORDLEVEL, "default", true},
    {46, 10, 10, WORDLEVEL, "default", true, 8, 3},
    {47, 10, 10, WORDLEVEL, "default", true, 128, 3},
    {48, 10, 10, WORDLEVEL, "realtime", true},
    {49, 10, 10, WORDLEVEL, "realtime", true, 8, 3},
    {50, 10, 10, WORDLEVEL, "realtime", true, 128, 3},
    {51, 10, 10, WORDLEVEL, "default:chunk", true},
    {52, 10, 10, WORDLEVEL, "default:chunk", true, 128, 3},
    {53, 10, 10, WORDLEVEL, "default:block", true},

    // 100
    {54, 100, 10, DOCLEVEL, "default", true},
    {55, 100, 10, DOCLEVEL, "default", true, 8, 3},
    {56, 100, 10, DOCLEVEL, "default", true, 128, 3},
    {57, 100, 10, DOCLEVEL, "realtime", true},
    {58, 100, 10, DOCLEVEL, "realtime", true, 8, 3},
    {59, 100, 10, DOCLEVEL, "realtime", true, 128, 3},
    {60, 100, 10, DOCLEVEL, "default:chunk", true},
    {61, 100, 10, DOCLEVEL, "default:chunk", true, 128, 3},
    {62, 100, 10, DOCLEVEL, "default:block", true},

    {63, 100, 10, WORDLEVEL, "default", true},
    {64, 100, 10, WORDLEVEL, "default", true, 8, 3},
    {65, 100, 10, WORDLEVEL, "default", true, 128, 3},
    {66, 100, 10, WORDLEVEL, "realtime", true},
    {67, 100, 10, WORDLEVEL, "realtime", true, 8, 3},
    {68, 100, 10, WORDLEVEL, "realtime", true, 128, 3},
    {69, 100, 10, WORDLEVEL, "default:chunk", true},
    {70, 100, 10, WORDLEVEL, "default:chunk", true, 128, 3},
    {71, 100, 10, WORDLEVEL, "default:block", true},

    // 1K
    {72, 1000, 10, DOCLEVEL, "default", true},
    {73, 1000, 10, DOCLEVEL, "default", true, 8, 3},
    {74, 1000, 10, DOCLEVEL, "default", true, 128, 3},
    {75, 1000, 10, DOCLEVEL, "realtime", true},
    {76, 1000, 10, DOCLEVEL, "realtime", true, 8, 3},
    {77, 1000, 10, DOCLEVEL, "realtime", true, 128, 3},
    {78, 1000, 10, DOCLEVEL, "default:chunk", true},
    {79, 1000, 10, DOCLEVEL, "default:chunk", true, 128, 3},
    {80, 1000, 10, DOCLEVEL, "default:block", true},

    {81, 1000, 10, WORDLEVEL, "default", true},
    {82, 1000, 10, WORDLEVEL, "default", true, 8, 3},
    {83, 1000, 10, WORDLEVEL, "default", true, 128, 3},
    {84, 1000, 10, WORDLEVEL, "realtime", true},
    {85, 1000, 10, WORDLEVEL, "realtime", true, 8, 3},
    {86, 1000, 10, WORDLEVEL, "realtime", true, 128, 3},
    {87, 1000, 10, WORDLEVEL, "default:chunk", true},
    {88, 1000, 10, WORDLEVEL, "default:chunk", true, 128, 3},
    {89, 1000, 10, WORDLEVEL, "default:block", true},

    // 10K
    {90, 10000, 10, DOCLEVEL, "default", true},
    {91, 10000, 10, DOCLEVEL, "default", true, 8, 3},
    {92, 10000, 10, DOCLEVEL, "default", true, 128, 3},
    {93, 10000, 10, DOCLEVEL, "realtime", true},
    {94, 10000, 10, DOCLEVEL, "realtime", true, 8, 3},
    {95, 10000, 10, DOCLEVEL, "realtime", true, 128, 3},
    {96, 10000, 10, DOCLEVEL, "default:chunk", true},
    {97, 10000, 10, DOCLEVEL, "default:chunk", true, 128, 3},
    {98, 10000, 10, DOCLEVEL, "default:block", true},

    {99,  10000, 10, WORDLEVEL, "default", true},
    {100, 10000, 10, WORDLEVEL, "default", true, 8, 3},
    {101, 10000, 10, WORDLEVEL, "default", true, 128, 3},
    {102, 10000, 10, WORDLEVEL, "realtime", true},
    {103, 10000, 10, WORDLEVEL, "realtime", true, 8, 3},
    {104, 10000, 10, WORDLEVEL, "realtime", true, 128, 3},
    {105, 10000, 10, WORDLEVEL, "default:chunk", true},
    {106, 10000, 10, WORDLEVEL, "default:chunk", true, 128, 3},
    {107, 10000, 10, WORDLEVEL, "default:block", true},

    // 100K
    {108, 100000, 10, DOCLEVEL, "default", true},
    {109, 100000, 10, DOCLEVEL, "default", true, 8, 3},
    {110, 100000, 10, DOCLEVEL, "default", true, 128, 3},
    {111, 100000, 10, DOCLEVEL, "realtime", true},
    {112, 100000, 10, DOCLEVEL, "realtime", true, 8, 3},
    {113, 100000, 10, DOCLEVEL, "realtime", true, 128, 3},
    {114, 100000, 10, DOCLEVEL, "default:chunk", true},
    {115, 100000, 10, DOCLEVEL, "default:chunk", true, 128, 3},
    {116, 100000, 10, DOCLEVEL, "default:block", true},

    {117, 100000, 10, WORDLEVEL, "default", true},
    {118, 100000, 10, WORDLEVEL, "default", true, 8, 3},
    {119, 100000, 10, WORDLEVEL, "default", true, 128, 3},
    {120, 100000, 10, WORDLEVEL, "realtime", true},
    {121, 100000, 10, WORDLEVEL, "realtime", true, 8, 3},
    {122, 100000, 10, WORDLEVEL, "realtime", true, 128, 3},
    {123, 100000, 10, WORDLEVEL, "default:chunk", true},
    {124, 100000, 10, WORDLEVEL, "default:chunk", true, 128, 3},
    {125, 100000, 10, WORDLEVEL, "default:block", true},

    // 1M
    {126, 1000000, 10, DOCLEVEL, "default", true},
    {127, 1000000, 10, DOCLEVEL, "default", true, 8, 3},
    {128, 1000000, 10, DOCLEVEL, "default", true, 128, 3},
    {129, 1000000, 10, DOCLEVEL, "realtime", true},
    {130, 1000000, 10, DOCLEVEL, "realtime", true, 8, 3},
    {131, 1000000, 10, DOCLEVEL, "realtime", true, 128, 3},
    {132, 1000000, 10, DOCLEVEL, "default:chunk", true},
    {133, 1000000, 10, DOCLEVEL, "default:chunk", true, 128, 3},
    {134, 1000000, 10, DOCLEVEL, "default:block", true},

    {135, 1000000, 10, WORDLEVEL, "default", true},
    {136, 1000000, 10, WORDLEVEL, "default", true, 8, 3},
    {137, 1000000, 10, WORDLEVEL, "default", true, 128, 3},
    {138, 1000000, 10, WORDLEVEL, "realtime", true},
    {139, 1000000, 10, WORDLEVEL, "realtime", true, 8, 3},
    {140, 1000000, 10, WORDLEVEL, "realtime", true, 128, 3},
    {141, 1000000, 10, WORDLEVEL, "default:chunk", true},
    {142, 1000000, 10, WORDLEVEL, "default:chunk", true, 128, 3},
    {143, 1000000, 10, WORDLEVEL, "default:block", true},


    // 10M
    {144, 10000000, 10, DOCLEVEL, "default", true},
    {145, 10000000, 10, DOCLEVEL, "default", true, 8, 3},
    {146, 10000000, 10, DOCLEVEL, "default", true, 128, 3},
    {147, 10000000, 10, DOCLEVEL, "realtime", true},
    {148, 10000000, 10, DOCLEVEL, "realtime", true, 8, 3},
    {149, 10000000, 10, DOCLEVEL, "realtime", true, 128, 3},
    {150, 10000000, 10, DOCLEVEL, "default:chunk", true},
    {151, 10000000, 10, DOCLEVEL, "default:chunk", true, 128, 3},
    {152, 10000000, 10, DOCLEVEL, "default:block", true},

    {153, 10000000, 10, WORDLEVEL, "default", true},
    {154, 10000000, 10, WORDLEVEL, "default", true, 8, 3},
    {155, 10000000, 10, WORDLEVEL, "default", true, 128, 3},
    {156, 10000000, 10, WORDLEVEL, "realtime", true},
    {157, 10000000, 10, WORDLEVEL, "realtime", true, 8, 3},
    {158, 10000000, 10, WORDLEVEL, "realtime", true, 128, 3},
    {159, 10000000, 10, WORDLEVEL, "default:chunk", true},
    {160, 10000000, 10, WORDLEVEL, "default:chunk", true, 128, 3},
    {161, 10000000, 10, WORDLEVEL, "default:block", true},
};

/** the parameter number */
const int INDEXER_TEST_CONFIG_NUM = sizeof(INDEXER_TEST_CONFIGS) / sizeof(IndexerTestConfig);

/** if the command options of "--run_config_list" or "--run_config_range" are not specified, run this config list */
const int DEFAULT_CONFIG_LIST[] = {18, 22, 24, 26, 28, 30};

/** the command option to specify a list of config numbers, such as "--run_config_list 0 1 2 3" */
const char* OPTION_CONFIG_LIST = "run_config_list";

/** the command option to specify a range of config numbers, such as "--run_config_range 4 7" */
const char* OPTION_CONFIG_RANGE = "run_config_range";

/**
 * the command option to specify a percentage of check statements to execute when @c IndexerTestConfig::docNum_ is not less than 1000.
 * such as "--check_percent 0.2", it would sample 20% check statements to execute.
 */
const char* OPTION_CHECK_PERCENT = "check_percent";

/** the default value for @c OPTION_CHECK_PERCENT. */
const double DEFAULT_CHECK_PERCENT = 0.2;

/** the environment variable name for boost test log */
const char* ENV_NAME_LOG_FILE = "BOOST_TEST_LOG_FILE";
}

static std::ofstream* gOutStream = 0;

template <typename ContainerT>
void loadConfigList(const ContainerT& configList, vector<IndexerTestConfig>& configVec)
{
    cout << OPTION_CONFIG_LIST << ": ";

    BOOST_FOREACH(int i, configList)
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

/**
 * Load command line option "run_config_list" or "run_config_range" to decide which configs to run.
 * @param configVec the loaded configs to run
 * @return true for load success, false for fail
 */
bool loadConfigOption(vector<IndexerTestConfig>& configVec)
{
    try
    {
        vector<int> runConfigListVec;
        vector<int> runConfigRangeVec;
        double checkPercent = DEFAULT_CHECK_PERCENT;
        po::options_description desp("Allowed options");
        desp.add_options()
            ("build_info,i", "this option is not used, it is just in case of boost test didn't filter out this option")
            (OPTION_CONFIG_LIST, po::value<std::vector<int> >(&runConfigListVec)->multitoken(), "specify the list of config parameters to run")
            (OPTION_CONFIG_RANGE, po::value<std::vector<int> >(&runConfigRangeVec)->multitoken(), "specify the range of config parameters to run")
            (OPTION_CHECK_PERCENT, po::value<double >(&checkPercent), "specify the percentage of check statements to execute for large data size (1000 docs)")
            ;

        po::options_description cmdline_options;
        cmdline_options.add(desp);

        po::variables_map vm;
        store(po::command_line_parser(framework::master_test_suite().argc, framework::master_test_suite().argv).options(cmdline_options).run(), vm);
        po::notify(vm);

        if(vm.count(OPTION_CONFIG_LIST))
        {
            loadConfigList(runConfigListVec, configVec);
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
            loadConfigList(DEFAULT_CONFIG_LIST, configVec);
        }

        cout << OPTION_CHECK_PERCENT << ": " << checkPercent << endl;
        BOOST_FOREACH(IndexerTestConfig& config, configVec)
            config.checkPercent_ = config.isDocNumLarge() ? checkPercent : 1.0;
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

    framework::master_test_suite().p_name.value = "t_integration_Indexer";

    const IndexerTestConfig* const pConfigStart = &configVec[0];
    const IndexerTestConfig* const pConfigEnd = pConfigStart + configVec.size();

    test_suite* tsIndexReader = BOOST_TEST_SUITE("t_IndexReader");
    tsIndexReader->add(BOOST_PARAM_TEST_CASE(&t_IndexReader::index, pConfigStart, pConfigEnd));
    tsIndexReader->add(BOOST_PARAM_TEST_CASE(&t_IndexReader::update, pConfigStart, pConfigEnd));
    tsIndexReader->add(BOOST_PARAM_TEST_CASE(&t_IndexReader::remove, pConfigStart, pConfigEnd));
    tsIndexReader->add(BOOST_PARAM_TEST_CASE(&t_IndexReader::empty, pConfigStart, pConfigEnd));
    framework::master_test_suite().add(tsIndexReader);

    test_suite* tsBarrelsInfo = BOOST_TEST_SUITE("t_BarrelsInfo");
    tsBarrelsInfo->add(BOOST_PARAM_TEST_CASE(&t_BarrelsInfo::index, pConfigStart, pConfigEnd));
    tsBarrelsInfo->add(BOOST_PARAM_TEST_CASE(&t_BarrelsInfo::optimize, pConfigStart, pConfigEnd));
    tsBarrelsInfo->add(BOOST_PARAM_TEST_CASE(&t_BarrelsInfo::createAfterOptimize, pConfigStart, pConfigEnd));
    tsBarrelsInfo->add(BOOST_PARAM_TEST_CASE(&t_BarrelsInfo::pauseResumeMerge, pConfigStart, pConfigEnd));
    tsBarrelsInfo->add(BOOST_PARAM_TEST_CASE(&t_BarrelsInfo::empty, pConfigStart, pConfigEnd));
    tsBarrelsInfo->add(BOOST_PARAM_TEST_CASE(&t_BarrelsInfo::resumeMergeAtStartUp, pConfigStart, pConfigEnd));
    framework::master_test_suite().add(tsBarrelsInfo);

    test_suite* tsTermDocFreqs = BOOST_TEST_SUITE("t_TermDocFreqs");
    tsTermDocFreqs->add(BOOST_PARAM_TEST_CASE(&t_TermDocFreqs::index, pConfigStart, pConfigEnd));
    tsTermDocFreqs->add(BOOST_PARAM_TEST_CASE(&t_TermDocFreqs::changeIndexMode, pConfigStart, pConfigEnd));
    tsTermDocFreqs->add(BOOST_PARAM_TEST_CASE(&t_TermDocFreqs::remove, pConfigStart, pConfigEnd));
    tsTermDocFreqs->add(BOOST_PARAM_TEST_CASE(&t_TermDocFreqs::update, pConfigStart, pConfigEnd));
    tsTermDocFreqs->add(BOOST_PARAM_TEST_CASE(&t_TermDocFreqs::empty, pConfigStart, pConfigEnd));
    tsTermDocFreqs->add(BOOST_PARAM_TEST_CASE(&t_TermDocFreqs::removeDocAndOptimize, pConfigStart, pConfigEnd));
    tsTermDocFreqs->add(BOOST_PARAM_TEST_CASE(&t_TermDocFreqs::optimizeAndRemoveDoc, pConfigStart, pConfigEnd));
    tsTermDocFreqs->add(BOOST_PARAM_TEST_CASE(&t_TermDocFreqs::removeOneTerm, pConfigStart, pConfigEnd));
    tsTermDocFreqs->add(BOOST_PARAM_TEST_CASE(&t_TermDocFreqs::checkRemoveAtStartUp, pConfigStart, pConfigEnd));
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
