///
/// @file t_CallProxy.cpp
/// @brief test CallProxy in calling member functions by string function name
/// @author Jun Jiang
/// @date Created 2012-03-29
///

#include <net/aggregator/BindCallProxyBase.h>

#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <iostream>

namespace
{

class StrUtil : public net::aggregator::BindCallProxyBase<StrUtil>
{
public:
    StrUtil(const std::string str) : str_(str) {}

    virtual bool bindCallProxy(CallProxyType& proxy)
    {
        BIND_CALL_PROXY_BEGIN(StrUtil, proxy)
        BIND_CALL_PROXY_1(get, std::string)
        BIND_CALL_PROXY_1(print, std::string)
        BIND_CALL_PROXY_2(size, std::string, int)
        BIND_CALL_PROXY_3(concat, std::string, int, std::string)
        BIND_CALL_PROXY_END()
    }

    void get(std::string& out)
    {
        out = str_;
    }

    void print(const std::string& in)
    {
        std::cout << "StrUtil::print() " << in << std::endl;
    }

    void size(const std::string& in, int& out)
    {
        out = in.size();
    }

    void concat(const std::string& in1, int in2, std::string& out)
    {
        out = in1 + boost::lexical_cast<std::string>(in2);
    }

private:
    std::string str_;
};

}

BOOST_AUTO_TEST_SUITE(t_CallProxy)

BOOST_AUTO_TEST_CASE(testProxy)
{
    std::string hello = "hello";

    StrUtil strUtil(hello);
    net::aggregator::CallProxy<StrUtil> proxy(&strUtil);
    BOOST_CHECK(strUtil.bindCallProxy(proxy));

    std::string result;
    BOOST_CHECK(proxy.call("get", result));
    BOOST_CHECK_EQUAL(result, hello);

    BOOST_CHECK(proxy.call("print", hello));

    int len = 0;
    BOOST_CHECK(proxy.call("size", hello, len));
    BOOST_CHECK_EQUAL(len, 5);

    int i = 123;
    BOOST_CHECK(proxy.call("concat", hello, i, result));
    BOOST_CHECK_EQUAL(result, "hello123");

    // duplicated binding
    BOOST_CHECK(strUtil.bindCallProxy(proxy) == false);

    // unknown function
    BOOST_CHECK(proxy.call("unknown", hello) == false);

    // signature not match
    BOOST_CHECK(proxy.call("print", len) == false);
    BOOST_CHECK(proxy.call("get", result, len) == false);
}

BOOST_AUTO_TEST_SUITE_END() 
