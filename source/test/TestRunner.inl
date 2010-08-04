#define BOOST_TEST_NO_MAIN
#define BOOST_TEST_ALTERNATIVE_INIT_API
#include <boost/test/unit_test.hpp>
#include <boost/test/output/xml_log_formatter.hpp>

#include "TestHelper.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

class MyXmlLogFormatter : public boost::unit_test::output::xml_log_formatter
{
    typedef boost::unit_test::output::xml_log_formatter super;

public:
    MyXmlLogFormatter()
    {
        std::cout.rdbuf(out.rdbuf());
        std::cerr.rdbuf(err.rdbuf());
    }

    void test_unit_start(std::ostream& ostr, boost::unit_test::test_unit const& tu)
    {
        out.str("");
        err.str("");
        super::test_unit_start(ostr, tu);
    }

    void test_unit_finish(std::ostream& ostr, boost::unit_test::test_unit const& tu, unsigned long elapsed)
    {
        if (tu.p_type == boost::unit_test::tut_case)
        {
            ostr << "<SystemOut><![CDATA[" << out.str() << "]]></SystemOut>";
            ostr << "<SystemErr><![CDATA[" << err.str() << "]]></SystemErr>";
        }

        super::test_unit_finish(ostr, tu, elapsed);
    }

private:
    std::ostringstream out;
    std::ostringstream err;
};

bool my_init_unit_test()
{
#ifdef BOOST_TEST_MODULE
    {
        using namespace ::boost::unit_test;
        assign_op( framework::master_test_suite().p_name.value, BOOST_TEST_STRINGIZE( BOOST_TEST_MODULE ).trim( "\"" ), 0 );
    }
#endif

    static std::ofstream ofs;

    const char* const gEnvName = "BOOST_TEST_LOG_FILE";
    const char* const gEnvValue = std::getenv(gEnvName);

    if (gEnvValue)
    {
        ofs.open(gEnvValue);
        boost::unit_test::unit_test_log.set_stream(ofs);
        boost::unit_test::unit_test_log.set_formatter(new MyXmlLogFormatter);
    }

    return true;
}

int BOOST_TEST_CALL_DECL
main(int argc, char* argv[])
{
    return boost::unit_test::unit_test_main(&my_init_unit_test, argc, argv);
}
