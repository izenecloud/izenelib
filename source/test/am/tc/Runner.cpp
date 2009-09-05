#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_NO_MAIN
#define BOOST_TEST_ALTERNATIVE_INIT_API

#include <boost/test/unit_test.hpp>

bool initRunner()
{
    using namespace ::boost::unit_test;
    framework::master_test_suite().p_name.value = "am::tc";

    return true;
}

int BOOST_TEST_CALL_DECL
main(int argc, char* argv[])
{
    return ::boost::unit_test::unit_test_main(&initRunner, argc, argv);
}
