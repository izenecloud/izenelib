#define BOOST_TEST_NO_MAIN
#define BOOST_TEST_ALTERNATIVE_INIT_API
#include <boost/test/unit_test.hpp>

#include "TestHelper.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include <util/izene_log.h>

static std::ofstream* gOutStream = 0;

bool my_init_unit_test()
{
#ifdef BOOST_TEST_MODULE
    {
        using namespace ::boost::unit_test;
        assign_op( framework::master_test_suite().p_name.value, BOOST_TEST_STRINGIZE( BOOST_TEST_MODULE ).trim( "\"" ), 0 );
    }
#endif

    const char* const gEnvName = "BOOST_TEST_LOG_FILE";
    const char* const gEnvValue = std::getenv(gEnvName);

    if (gEnvValue)
    {
        gOutStream = new std::ofstream(gEnvValue);
        boost::unit_test::unit_test_log.set_stream(*gOutStream);

        boost::unit_test::unit_test_log.set_formatter(new MyXmlLogFormatter());
    }

    return true;
}

int BOOST_TEST_CALL_DECL
main(int argc, char* argv[])
{
#ifdef INIT_GLOG
    google::InitGoogleLogging(argv[0]);
#endif
    int status = boost::unit_test::unit_test_main(&my_init_unit_test, argc, argv);

    // delete global stream to flush
    delete gOutStream;

    return status;
}
