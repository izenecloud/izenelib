/**
 * @file TestHelper.h
 *
 * Created      <2008-10-19 17:59:22 Ian Yang>
 * Last Updated <2009-02-27 18:04:05 Ian Yang>
 */
#ifndef INCLUDED_TEST_HELPER_H
#define INCLUDED_TEST_HELPER_H

#define IZS_CHECK_DOUBLE_CLOSE(a, b, e) BOOST_CHECK_SMALL((static_cast<double>(a) - static_cast<double>(b)), static_cast<double>(e))

#include <boost/version.hpp>

#if BOOST_VERSION >= 103500 && BOOST_VERSION <= 103700

#include <boost/test/test_tools.hpp>
#include <boost/test/floating_point_comparison.hpp>

namespace {

struct ignore_unused_variable_warnings_in_boost_test
{
    ignore_unused_variable_warnings_in_boost_test()
    {
        use(boost::test_tools::check_is_small);
        use(boost::test_tools::dummy_cond);
        use(boost::test_tools::check_is_close);
    }

    template<typename T>
    void use(const T&)
    {
        // empty
    }
};

}

#endif

#include <boost/test/output/xml_log_formatter.hpp>
#include <boost/test/unit_test.hpp>
#include <iostream>

class MyXmlLogFormatter : public boost::unit_test::output::xml_log_formatter
{
    typedef boost::unit_test::output::xml_log_formatter super;

public:
    MyXmlLogFormatter()
    : outBackup_(0), errBackup_(0)
    {
        outBackup_ = std::cout.rdbuf(out_.rdbuf());
        errBackup_ = std::cerr.rdbuf(err_.rdbuf());
    }

    ~MyXmlLogFormatter()
    {
        if (outBackup_)
        {
            std::cout.rdbuf(outBackup_);
        }
        if (errBackup_)
        {
            std::cerr.rdbuf(errBackup_);
        }
    }

    void test_unit_start(std::ostream& ostr, boost::unit_test::test_unit const& tu)
    {
        out_.str("");
        err_.str("");
        super::test_unit_start(ostr, tu);
    }

    void test_unit_finish(std::ostream& ostr, boost::unit_test::test_unit const& tu, unsigned long elapsed)
    {
        if (tu.p_type == boost::unit_test::tut_case)
        {
            ostr << "<SystemOut><![CDATA[" << out_.str() << "]]></SystemOut>";
            ostr << "<SystemErr><![CDATA[" << err_.str() << "]]></SystemErr>";
        }

        super::test_unit_finish(ostr, tu, elapsed);
    }

private:
    std::ostringstream out_;
    std::ostringstream err_;
    std::streambuf* outBackup_;
    std::streambuf* errBackup_;
};
#endif // INCLUDED_TEST_HELPER_H
