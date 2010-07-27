#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>

#include "TestHelper.h"

#include <fstream>
#include <cstdlib>

int BOOST_TEST_CALL_DECL
main(int argc, char* argv[])
{
    const char* const gEnvName = "BOOST_TEST_LOG_FILE";
    const char* const gEnvValue = std::getenv(gEnvName);

    std::ofstream ofs;
    if (gEnvValue)
    {
        ofs.open(gEnvValue);
        ::boost::unit_test::unit_test_log.set_stream(ofs);
    }

    return ::boost::unit_test::unit_test_main(&init_unit_test, argc, argv);
}
